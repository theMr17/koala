#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <vector>

std::wstring toW(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring out(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], n);
    return out;
}

std::string readResponse(HINTERNET hRequest) {
    std::string result;
    DWORD dwSize = 0;
    do {
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
        if (dwSize == 0) break;
        std::vector<char> buffer(dwSize);
        DWORD dwDownloaded = 0;
        if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded) && dwDownloaded) {
            result.append(buffer.data(), buffer.data() + dwDownloaded);
        } else break;
    } while (dwSize > 0);
    return result;
}

bool httpGet(const std::wstring& host, INTERNET_PORT port, const std::wstring& path) {
    HINTERNET hSession = WinHttpOpen(L"Koala/0.1",
                                     WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { std::cerr << "WinHttpOpen failed\n"; return false; }

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) { std::cerr << "WinHttpConnect failed\n"; WinHttpCloseHandle(hSession); return false; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
                                           nullptr, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           (port == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) { std::cerr << "WinHttpOpenRequest failed\n"; WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }

    BOOL bResult = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!bResult) {
        std::cerr << "WinHttpSendRequest failed\n";
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return false;
    }

    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        std::cerr << "WinHttpReceiveResponse failed\n";
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return false;
    }

    // Get status code
    DWORD status = 0; DWORD sz = sizeof(status);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);
    std::cout << "Status: " << status << "\n";

    std::string body = readResponse(hRequest);
    std::cout << "Body:\n" << body << "\n";

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return true;
}

bool httpPostJson(const std::wstring& host, INTERNET_PORT port, const std::wstring& path, const std::string& jsonBody) {
    HINTERNET hSession = WinHttpOpen(L"Koala/0.1",
                                     WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { std::cerr << "WinHttpOpen failed\n"; return false; }

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) { std::cerr << "WinHttpConnect failed\n"; WinHttpCloseHandle(hSession); return false; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
                                           nullptr, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES,
                                           (port == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) { std::cerr << "WinHttpOpenRequest failed\n"; WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }

    std::wstring headers = L"Content-Type: application/json\r\n";
    BOOL bResult = WinHttpSendRequest(hRequest,
                                      headers.c_str(), (DWORD)headers.size(),
                                      (LPVOID)jsonBody.c_str(), (DWORD)jsonBody.size(),
                                      (DWORD)jsonBody.size(), 0);
    if (!bResult) {
        std::cerr << "WinHttpSendRequest failed\n";
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return false;
    }

    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        std::cerr << "WinHttpReceiveResponse failed\n";
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD status = 0; DWORD sz = sizeof(status);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);
    std::cout << "Status: " << status << "\n";

    std::string body = readResponse(hRequest);
    std::cout << "Body:\n" << body << "\n";

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return true;
}

int main() {
    // Example usage:
    // GET https://jsonplaceholder.typicode.com/posts/1
    httpGet(L"jsonplaceholder.typicode.com", INTERNET_DEFAULT_HTTPS_PORT, L"/posts/1");

    // POST example
    std::string json = R"({"title":"foo","body":"bar","userId":1})";
    httpPostJson(L"jsonplaceholder.typicode.com", INTERNET_DEFAULT_HTTPS_PORT, L"/posts", json);

    return 0;
}
