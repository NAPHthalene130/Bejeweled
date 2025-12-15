#include "SqlUtil.h"
#include <iostream>

// Stubbed implementations used when MySQL support is disabled via CMake option

void SqlUtil::testConnection() {
    std::cout << "SqlUtil: MySQL disabled (stub). testConnection skipped." << std::endl;
}

std::string SqlUtil::getPlayerPasswordByPlayerIDfromPlayerinfo(std::string) { return std::string(); }
std::string SqlUtil::getEmailByPlayerIDfromPlayerinfo(std::string) { return std::string(); }
std::string SqlUtil::getStyleSetByPlayerIDfromPlayerinfo(std::string) { return std::string(); }
std::string SqlUtil::getSaltByPlayerIDfromPlayerinfo(std::string) { return std::string(); }
int SqlUtil::getIterationsByPlayerIDfromPlayerinfo(std::string) { return 0; }

void SqlUtil::setPlayerPasswordByPlayerIDfromPlayerinfo(std::string, std::string) {}
void SqlUtil::setEmailByPlayerIDfromPlayerinfo(std::string, std::string) {}
void SqlUtil::setStyleSetByPlayerIDfromPlayerinfo(std::string, std::string) {}
void SqlUtil::setSaltByPlayerIDfromPlayerinfo(std::string, std::string) {}
void SqlUtil::setIterationsByPlayerIDfromPlayerinfo(std::string, int) {}

bool SqlUtil::authEmailCode(std::string, std::string) { return false; }
int SqlUtil::authPasswordFromPlayerinfo(std::string, std::string) { return 3; }
int SqlUtil::registerFromPlayerinfo(std::string, std::string, std::string, std::string, std::string) { return 5; }
