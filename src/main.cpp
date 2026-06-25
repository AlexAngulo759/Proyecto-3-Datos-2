#include "WebApi/ApiServer.h"
#include <iostream>

int main() {
    try {
        ApiServer server;

        std::cout << "TinySQL API running on port 8080..." << std::endl;

        server.start(8080);
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    return 0;
}