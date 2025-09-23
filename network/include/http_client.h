#pragma once
#include <memory>
#include <system_error>
#include <utility>
#include "http_request.h"
#include "http_response.h"

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
        virtual std::pair<HttpResponse, std::error_code> send(const HttpRequest &req) = 0;
    };

    /**
     * Create the platform-default client.
     * Implementation should live in a platform-specific .cpp
     * (e.g. network/src/windows_client.cpp on Windows)
     * so public headers don't need platform-specific includes.
     */
    std::unique_ptr<IHttpClient> createDefaultHttpClient();
} 
