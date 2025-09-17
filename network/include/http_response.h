#pragma once
#include <string>
#include <map>
#include <optional>
#include <vector>

namespace koala::network {
    /**
     * Basic HTTP response container.
     */
    struct HttpResponse {
        int status_code = 0;
        std::string status_text; // optional reason phrase if available
        std::map<std::string, std::string> headers; // response headers
        std::string body; // response body (binary-safe stored in std::string)

        bool ok() const noexcept { return status_code >= 200 && status_code < 300; }

        std::optional<std::string> header(const std::string& name) const {
            const auto it = headers.find(name);
            if (it == headers.end()) return std::nullopt;
            return it->second;
        }
    };
}
