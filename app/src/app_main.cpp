#include <iostream>

#include "application.h"
#include "http_client.h"
#include "storage.h"

namespace koala::core {
    int runWindowDemo();
}

/**
 * @brief Program entry point that performs an HTTP GET request, prints the response, and starts the application run loop.
 *
 * This function creates a default HTTP client, sends a GET request to
 * "https://jsonplaceholder.typicode.com/posts/1", outputs the response status
 * code and body to standard output, and then constructs and runs the
 * koala::core::Application instance.
 *
 * @return int Exit status: `0` on success; `1` if the HTTP request fails (error message is written to standard error).
 */
int main() {
    const auto client = koala::network::createDefaultHttpClient();

    koala::network::HttpRequest req;
    req.method = "GET";
    req.url = "https://jsonplaceholder.typicode.com/posts/1";

    auto [resp, err] = client->send(req);

    if (err) {
        std::cerr << "Error: " << err.message() << "\n";
        return 1;
    }

    std::cout << "Status: " << resp.status_code << "\n";
    std::cout << "Body:\n" << resp.body << "\n";

    koala::storage::print();

    koala::core::Application application;
    application.Run();
    return 0;
}
