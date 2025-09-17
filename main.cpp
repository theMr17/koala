#include <iostream>
#include "network/include/http_client.h"
#include "network/include/http_request.h"

int main() {
    auto client = koala::network::createDefaultHttpClient();

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
    return 0;
}
