#include <iostream>
#include "http_client.h"

namespace koala::core {
    int runWindowDemo();
}

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

    return koala::core::runWindowDemo();
}
