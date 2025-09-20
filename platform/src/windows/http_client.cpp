#include <windows.h>
#include <winhttp.h>
#include "http_client.h"
#include "utils.h"

namespace koala::platform::windows {
    // Helper: Read HTTP response body
    static std::string readResponse(HINTERNET hRequest) {
        std::string result;
        DWORD dwSize = 0;
        do {
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
            if (dwSize == 0) break;

            std::vector<char> buffer(dwSize);
            DWORD dwDownloaded = 0;
            if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded) && dwDownloaded > 0) {
                result.append(buffer.data(), dwDownloaded);
            } else break;
        } while (dwSize > 0);
        return result;
    }

    // Concrete Windows HTTP client
    class WinHttpClient : public network::IHttpClient {
    public:
        std::pair<network::HttpResponse, std::error_code> send(const network::HttpRequest &request) override {
            network::HttpResponse response;

            const HINTERNET hSession = WinHttpOpen(L"Koala/0.1",
                                             WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                             WINHTTP_NO_PROXY_NAME,
                                             WINHTTP_NO_PROXY_BYPASS, 0);
            if (!hSession) {
                return {{}, std::error_code(GetLastError(), std::system_category())};
            }

            // Parse URL into host + path
            URL_COMPONENTS components = {};
            components.dwStructSize = sizeof(components);
            std::wstring wurl = core::util::utf8ToWide(request.url);
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

            std::wstring method = core::util::utf8ToWide(request.method);
            HINTERNET hRequest = WinHttpOpenRequest(
                hConnect, method.c_str(), path.c_str(),
                nullptr, WINHTTP_NO_REFERER,
                WINHTTP_DEFAULT_ACCEPT_TYPES,
                (components.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0
            );
            if (!hRequest) {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return {{}, std::error_code(GetLastError(), std::system_category())};
            }

            // Build headers
            std::wstring headers;
            for (auto &[k, v]: request.headers) {
                headers += core::util::utf8ToWide(k + ": " + v + "\r\n");
            }

            BOOL bResult = WinHttpSendRequest(
                hRequest,
                headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
                headers.empty() ? 0 : (DWORD) headers.size(),
                request.body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID) request.body.c_str(),
                (DWORD) request.body.size(),
                (DWORD) request.body.size(),
                0
            );

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
            if (WinHttpQueryHeaders(hRequest,
                                    WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                    WINHTTP_HEADER_NAME_BY_INDEX,
                                    &status, &sz, WINHTTP_NO_HEADER_INDEX)) {
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

// Factory function (declared in http_client.h)
namespace koala::network {
    std::unique_ptr<IHttpClient> createDefaultHttpClient() {
        return std::make_unique<platform::windows::WinHttpClient>();
    }
}
