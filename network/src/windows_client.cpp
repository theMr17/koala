#include <future>
#include <windows.h>
#include <winhttp.h>
#include <vector>
#include <string>
#include "http_client.h"
#include "utils.h"

namespace koala::platform::windows {
    // Helper: Read HTTP response body (synchronous path)
    static std::string readResponse(HINTERNET hRequest) {
        std::string result;
        DWORD dwSize = 0;
        do {
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                break;
            if (dwSize == 0)
                break;

            std::vector<char> buffer(dwSize);
            DWORD dwDownloaded = 0;
            if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded) && dwDownloaded > 0) {
                result.append(buffer.data(), dwDownloaded);
            } else
                break;
        } while (dwSize > 0);
        return result;
    }

    // Concrete Windows HTTP client (blocking)
    class WinHttpClient : public network::IHttpClient {
    public:
        std::pair<network::HttpResponse, std::error_code> send(const network::HttpRequest &request) override {
            network::HttpResponse response;

            const HINTERNET hSession = WinHttpOpen(L"Koala/0.1", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                                   WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) {
                return {{}, std::error_code(GetLastError(), std::system_category())};
            }

            // Parse URL into host + path
            URL_COMPONENTS components = {};
            components.dwStructSize = sizeof(components);
            std::wstring wurl = koala::core::util::utf8ToWide(request.url);
            components.dwHostNameLength = (DWORD) -1;
            components.dwUrlPathLength = (DWORD) -1;

            if (!WinHttpCrackUrl(wurl.c_str(), (DWORD) wurl.size(), 0, &components)) {
                WinHttpCloseHandle(hSession);
                return {{}, std::error_code(GetLastError(), std::system_category())};
            }

            std::wstring host(components.lpszHostName, components.dwHostNameLength);
            std::wstring path(components.lpszUrlPath, components.dwUrlPathLength);

            HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), components.nPort, 0);
            if (!hConnect) {
                WinHttpCloseHandle(hSession);
                return {{}, std::error_code(GetLastError(), std::system_category())};
            }

            std::wstring method = koala::core::util::utf8ToWide(request.method);
            HINTERNET hRequest = WinHttpOpenRequest(
                    hConnect, method.c_str(), path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                    (components.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);
            if (!hRequest) {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return {{}, std::error_code(GetLastError(), std::system_category())};
            }

            if (request.timeout_ms.has_value()) {
                int t = *request.timeout_ms;
                WinHttpSetTimeouts(hRequest, t, t, t, t);
            }

            // Build headers
            std::wstring headers;
            for (auto &[k, v]: request.headers) {
                headers += koala::core::util::utf8ToWide(k + ": " + v + "\r\n");
            }

            BOOL bResult = WinHttpSendRequest(
                    hRequest, headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
                    headers.empty() ? 0 : (DWORD) headers.size(),
                    request.body.empty() ? WINHTTP_NO_REQUEST_DATA
                                         : reinterpret_cast<LPVOID>(const_cast<char *>(request.body.data())),
                    (DWORD)request.body.size(), (DWORD)request.body.size(), 0);

            if (!bResult) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return {{}, std::error_code(GetLastError(), std::system_category())};
            }

            if (!WinHttpReceiveResponse(hRequest, nullptr)) {
                WinHttpCloseHandle(hRequest);
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return {{}, std::error_code(GetLastError(), std::system_category())};
            }

            // Get status code
            DWORD status = 0;
            DWORD sz = sizeof(status);
            if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                    WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX)) {
                response.status_code = static_cast<int>(status);
            }

            // Read body
            response.body = readResponse(hRequest);

            // Cleanup
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);

            return {response, {}}; // success → no error
        }
    };

} // namespace koala::platform::windows

namespace koala::network {
    using namespace koala::platform::windows;

    // True asynchronous implementation using WinHTTP async callbacks
    // State shared with the WinHTTP callback; freed on completion.
    struct WinHttpAsyncContext {
        HINTERNET hSession = nullptr;
        HINTERNET hConnect = nullptr;
        HINTERNET hRequest = nullptr;

        std::promise<NetResult> promise;
        HttpResponse response;
        std::wstring headersW;
        std::vector<char> readBuffer; // buffer for async reads
        DWORD available = 0;          // for WinHttpQueryDataAvailable
        bool completed = false;
    };

    static void close_all(WinHttpAsyncContext* ctx) {
        if (!ctx) return;
        if (ctx->hRequest) { WinHttpCloseHandle(ctx->hRequest); ctx->hRequest = nullptr; }
        if (ctx->hConnect) { WinHttpCloseHandle(ctx->hConnect); ctx->hConnect = nullptr; }
        if (ctx->hSession) { WinHttpCloseHandle(ctx->hSession); ctx->hSession = nullptr; }
    }

    static void fulfill_and_cleanup(WinHttpAsyncContext* ctx, std::error_code ec) {
        if (!ctx) return;
        if (!ctx->completed) {
            ctx->completed = true;
            NetResult r{ std::move(ctx->response), ec };
            ctx->promise.set_value(std::move(r));
        }
        close_all(ctx);
        delete ctx;
    }

    static void CALLBACK StatusCallback(HINTERNET hHandle,
                                        DWORD_PTR dwContext,
                                        DWORD dwInternetStatus,
                                        LPVOID lpvStatusInformation,
                                        DWORD dwStatusInformationLength) {
        auto* ctx = reinterpret_cast<WinHttpAsyncContext*>(dwContext);
        if (!ctx) return;

        switch (dwInternetStatus) {
            case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE: {
                // Begin receiving response headers
                if (!WinHttpReceiveResponse(ctx->hRequest, nullptr)) {
                    DWORD err = GetLastError();
                    if (err != ERROR_IO_PENDING) {
                        fulfill_and_cleanup(ctx, std::error_code((int)err, std::system_category()));
                    }
                }
                break;
            }
            case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: {
                // Extract status code
                DWORD status = 0; DWORD sz = sizeof(status);
                if (WinHttpQueryHeaders(ctx->hRequest,
                                        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                        WINHTTP_HEADER_NAME_BY_INDEX,
                                        &status, &sz, WINHTTP_NO_HEADER_INDEX)) {
                    ctx->response.status_code = (int)status;
                }
                // Start async body retrieval
                if (!WinHttpQueryDataAvailable(ctx->hRequest, &ctx->available)) {
                    DWORD err = GetLastError();
                    if (err != ERROR_IO_PENDING) {
                        fulfill_and_cleanup(ctx, std::error_code((int)err, std::system_category()));
                    }
                }
                break;
            }
            case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE: {
                // lpvStatusInformation points to a DWORD with available size
                DWORD available = (dwStatusInformationLength == sizeof(DWORD) && lpvStatusInformation)
                                  ? *reinterpret_cast<DWORD*>(lpvStatusInformation)
                                  : ctx->available;
                if (available == 0) {
                    // Done
                    fulfill_and_cleanup(ctx, {});
                    break;
                }
                ctx->readBuffer.resize(available);
                DWORD bytesRead = 0; // we don't need this value here; provided in READ_COMPLETE
                if (!WinHttpReadData(ctx->hRequest,
                                     ctx->readBuffer.data(),
                                     available,
                                     &bytesRead)) {
                    DWORD err = GetLastError();
                    if (err != ERROR_IO_PENDING) {
                        fulfill_and_cleanup(ctx, std::error_code((int)err, std::system_category()));
                    }
                }
                break;
            }
            case WINHTTP_CALLBACK_STATUS_READ_COMPLETE: {
                // Append to body
                if (lpvStatusInformation && dwStatusInformationLength > 0) {
                    ctx->response.body.append(static_cast<const char*>(lpvStatusInformation), dwStatusInformationLength);
                }
                // Query for more data
                if (!WinHttpQueryDataAvailable(ctx->hRequest, &ctx->available)) {
                    DWORD err = GetLastError();
                    if (err != ERROR_IO_PENDING) {
                        fulfill_and_cleanup(ctx, std::error_code((int)err, std::system_category()));
                    }
                }
                break;
            }
            case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: {
                auto* ar = reinterpret_cast<WINHTTP_ASYNC_RESULT*>(lpvStatusInformation);
                DWORD err = ar ? ar->dwError : GetLastError();
                fulfill_and_cleanup(ctx, std::error_code((int)err, std::system_category()));
                break;
            }
            case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING: {
                // No action needed; cleanup handled in fulfill
                break;
            }
            default:
                break;
        }
    }

    std::future<NetResult> send_async_future(HttpRequest req) {
        // Allocate context on heap; it will self-destroy on completion via callback
        auto* ctx = new WinHttpAsyncContext();

        // Open async session
        ctx->hSession = WinHttpOpen(L"Koala/0.1",
                                    WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS,
                                    WINHTTP_FLAG_ASYNC);
        if (!ctx->hSession) {
            auto fut = ctx->promise.get_future();
            std::error_code ec((int)GetLastError(), std::system_category());
            fulfill_and_cleanup(ctx, ec);
            return fut;
        }

        // Parse URL
        URL_COMPONENTS comp{}; comp.dwStructSize = sizeof(comp);
        std::wstring wurl = koala::core::util::utf8ToWide(req.url);
        comp.dwHostNameLength = (DWORD)-1;
        comp.dwUrlPathLength = (DWORD)-1;
        if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.size(), 0, &comp)) {
            auto fut = ctx->promise.get_future();
            std::error_code ec((int)GetLastError(), std::system_category());
            fulfill_and_cleanup(ctx, ec);
            return fut;
        }
        std::wstring host(comp.lpszHostName, comp.dwHostNameLength);
        std::wstring path(comp.lpszUrlPath, comp.dwUrlPathLength);

        ctx->hConnect = WinHttpConnect(ctx->hSession, host.c_str(), comp.nPort, 0);
        if (!ctx->hConnect) {
            auto fut = ctx->promise.get_future();
            std::error_code ec((int)GetLastError(), std::system_category());
            fulfill_and_cleanup(ctx, ec);
            return fut;
        }

        std::wstring method = koala::core::util::utf8ToWide(req.method);
        ctx->hRequest = WinHttpOpenRequest(
                ctx->hConnect, method.c_str(), path.c_str(), nullptr,
                WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                (comp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);
        if (!ctx->hRequest) {
            auto fut = ctx->promise.get_future();
            std::error_code ec((int)GetLastError(), std::system_category());
            fulfill_and_cleanup(ctx, ec);
            return fut;
        }

        // Optional timeouts
        if (req.timeout_ms.has_value()) {
            int t = *req.timeout_ms;
            WinHttpSetTimeouts(ctx->hRequest, t, t, t, t);
        }

        // Set callback and context value
        WINHTTP_STATUS_CALLBACK prev = WinHttpSetStatusCallback(
                ctx->hRequest,
                (WINHTTP_STATUS_CALLBACK)StatusCallback,
                WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE |
                WINHTTP_CALLBACK_FLAG_HEADERS_AVAILABLE   |
                WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE      |
                WINHTTP_CALLBACK_FLAG_READ_COMPLETE       |
                WINHTTP_CALLBACK_FLAG_REQUEST_ERROR,
                0);
        (void)prev;
        DWORD_PTR contextVal = reinterpret_cast<DWORD_PTR>(ctx);
        WinHttpSetOption(ctx->hRequest, WINHTTP_OPTION_CONTEXT_VALUE, &contextVal, sizeof(contextVal));

        // Build headers
        std::wstring headersW;
        for (auto &kv : req.headers) {
            headersW += koala::core::util::utf8ToWide(kv.first + ": " + kv.second + "\r\n");
        }

        // Kick off async send
        BOOL ok = WinHttpSendRequest(
                ctx->hRequest,
                headersW.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headersW.c_str(),
                headersW.empty() ? 0 : (DWORD)headersW.size(),
                req.body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)req.body.data(),
                (DWORD)req.body.size(), (DWORD)req.body.size(),
                0);
        if (!ok) {
            DWORD err = GetLastError();
            if (err != ERROR_IO_PENDING) {
                auto fut = ctx->promise.get_future();
                fulfill_and_cleanup(ctx, std::error_code((int)err, std::system_category()));
                return fut;
            }
        }

        return ctx->promise.get_future();
    }

    std::unique_ptr<IHttpClient> createDefaultHttpClient() {
        return std::make_unique<platform::windows::WinHttpClient>();
    }
} // namespace koala::network
