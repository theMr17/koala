#pragma once
#include <string>
#include <map>
#include <optional>
#include <vector>
#include "utils.h"

namespace koala::network {
    /**
     * Basic HTTP response container.
     */
    struct HttpResponse {
        int status_code = 0;
        std::string status_text; // optional reason phrase if available
        std::map<std::string, std::string> headers; // response headers (header name matching is case-insensitive)
        std::string body; // response body (binary-safe stored in std::string)

        bool ok() const noexcept { return status_code >= 200 && status_code < 300; }

        // Case-insensitive lookup per HTTP spec
        std::optional<std::string> header(const std::string& name) const {
            return koala::core::util::findHeaderCaseInsensitive(headers, name);
        }
    };
}
