#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
public:
    // Server IP Address (IPv6)
    static std::string getServerIp() {
        return "127.0.0.1";
    }

    // Authentication Port
    static int getAuthPort() {
        return 10086;
    }

    // Other Net Data Port
    static int getOtherNetDataPort() {
        return 10088;
    }

    // Game Net Data Port
    static std::string getGameNetDataPort() {
        return "10090";
    }
};

#endif // CONFIG_H
