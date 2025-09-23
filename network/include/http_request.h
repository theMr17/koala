#pragma once
#include <string>
#include <map>
#include <optional>
#include "utils.h"

namespace koala::network {
    /**
     * Simple representation of an HTTP request.
     * This is a POD-like class purposely kept small and copyable.
     */
    struct HttpRequest {
        std::string method = "GET"; // "GET", "POST", "PUT", "DELETE", etc.
        std::string url; // full URL (e.g. "https://example.com/api/1")
        std::map<std::string, std::string> headers; // header-name -> header-value (header name matching is case-insensitive)
        std::string body; // request body (for POST/PUT)
        std::optional<int> timeout_ms; // optional timeout in milliseconds

        // Helpers
        void setHeader(const std::string& name, const std::string& value) {
            headers[name] = value;
        }

        // Case-insensitive lookup per HTTP spec
        std::optional<std::string> getHeader(const std::string& name) const {
            return koala::core::util::findHeaderCaseInsensitive(headers, name);
        }
    };
}
