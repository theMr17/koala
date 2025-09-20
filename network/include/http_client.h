#pragma once
#include "http_request.h"
#include "http_response.h"
#include <memory>

namespace koala::network {
    /**
     * Abstract HTTP client interface.
     *
     * Implement platform-specific clients (e.g. WinHttpClient) by deriving from this.
     * Keep this header free of platform-specific includes.
     */
    class IHttpClient {
    public:
        virtual ~IHttpClient() = default;

        /**
         * Send a request synchronously and return the response.
         * On failure, returns a non-empty std::error_code.
         */
        virtual std::pair<HttpResponse, std::error_code> send(const HttpRequest& req) = 0;
    };

    /**
     * Create the platform-default client.
     * Implementation should live in platform/ (e.g. platform/windows/http_client_win.cpp)
     * so public headers don't need platform-specific includes.
     */
    std::unique_ptr<IHttpClient> createDefaultHttpClient();
}
