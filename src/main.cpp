#include <iostream>
#include "authServer/LoginServer.h"

int main() {
    try {
        unsigned short port = 10086;
        LoginServer server(port);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
