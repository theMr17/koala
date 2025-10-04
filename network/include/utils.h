#pragma once
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

namespace koala::core::util {
    /**
     * Minimal URL parsing result.
     * Only stores pieces you typically need for HTTP clients.
     *
     * NOTE: This parser is simple and covers common URLs. If you need
     * full RFC-compliant parsing, swap in a dedicated parser later.
     */
    struct UrlParts {
        std::string scheme; // "http" or "https"
        std::string host; // host or IP
        uint16_t port = 0; // 80 or 443 if not specified
        std::string path; // path + query, e.g. "/api/v1?x=1"
    };

    /**
     * Parse a URL into UrlParts. Returns std::nullopt on failure.
     * Examples:
     *  - https://example.com/path -> scheme=https host=example.com port=443 path=/path
     *  - http://localhost:8080/   -> port=8080
     */
    inline std::optional<UrlParts> parseUrl(const std::string &url) {
        // Very small parser for common http(s) URLs.
        // Not fully RFC compliant — enough for Koala's needs.
        auto lower = url;
        std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return std::tolower(c); });

        const std::string http_prefix = "http://";
        const std::string https_prefix = "https://";
        UrlParts out;

        size_t pos = std::string::npos;
        if (lower.rfind(https_prefix, 0) == 0) {
            out.scheme = "https";
            pos = https_prefix.size();
            out.port = 443;
        } else if (lower.rfind(http_prefix, 0) == 0) {
            out.scheme = "http";
            pos = http_prefix.size();
            out.port = 80;
        } else {
            return std::nullopt;
        }

        // host[:port][path...]
        size_t host_end = url.find_first_of("/:?", pos);
        if (host_end == std::string::npos) {
            out.host = url.substr(pos);
            out.path = "/";
            return out;
        }

        out.host = url.substr(pos, host_end - pos);

        // check for :port
        if (url.size() > host_end && url[host_end] == ':') {
            size_t port_start = host_end + 1;
            size_t port_end = url.find_first_of("/?", port_start);
            std::string port_str = (port_end == std::string::npos) ? url.substr(port_start)
                                                                   : url.substr(port_start, port_end - port_start);
            try {
                int p = std::stoi(port_str);
                if (p > 0 && p <= 65535)
                    out.port = static_cast<uint16_t>(p);
            } catch (...) {
                return std::nullopt;
            }
            host_end = (port_end == std::string::npos) ? url.size() : port_end;
        }

        // path and query
        if (host_end < url.size()) {
            out.path = url.substr(host_end);
        } else {
            out.path = "/";
        }

        return out;
    }

    /**
     * Case-insensitive header map lookup helper.
     */
    inline std::optional<std::string> findHeaderCaseInsensitive(const std::map<std::string, std::string> &headers,
                                                                const std::string &name) {
        auto lname = name;
        std::ranges::transform(lname, lname.begin(), [](unsigned char c) { return std::tolower(c); });
        for (const auto &[k, v]: headers) {
            auto kk = k;
            std::ranges::transform(kk, kk.begin(), [](unsigned char c) { return std::tolower(c); });
            if (kk == lname)
                return v;
        }
        return std::nullopt;
    }

#ifdef _WIN32
    /**
     * Convert UTF-8 std::string to std::wstring (Windows helper).
     */
    inline std::wstring utf8ToWide(const std::string &s) {
        if (s.empty())
            return {};
        int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
        std::wstring out(n, 0);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), &out[0], n);
        return out;
    }
#endif
} // namespace koala::core::util
