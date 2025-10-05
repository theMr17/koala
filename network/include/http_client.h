#pragma once
#include <memory>
#include <system_error>
#include <utility>
#include <future>
#include "http_request.h"
#include "http_response.h"

namespace koala::network {
    /**
     * Simple result type for asynchronous HTTP operations.
     */
    struct NetResult {
        HttpResponse response;
        std::error_code error;
    };

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
     * Fire a request using the platform's asynchronous HTTP implementation.
     * Returns a future that resolves when the request completes (or fails).
     * On platforms without a native async backend, it may fall back to a worker thread.
     */
    std::future<NetResult> send_async_future(HttpRequest req);

    /**
     * Create the platform-default client.
     * Implementation should live in a platform-specific .cpp
     * (e.g. network/src/windows_client.cpp on Windows)
     * so public headers don't need platform-specific includes.
     */
    std::unique_ptr<IHttpClient> createDefaultHttpClient();
} 
