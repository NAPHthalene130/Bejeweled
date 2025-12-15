#include "Auth/AuthWindow.h"
#include <QApplication>
#include <iostream>
#include "utils/ResourceUtils.h"

extern "C" {
    __declspec(dllimport) int __stdcall WSAStartup(unsigned short, void*);
    __declspec(dllimport) int __stdcall WSACleanup();
}

#pragma comment(lib, "ws2_32")
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Verify resource path
    std::cout << "Current Resource Path: " << ResourceUtils::getResourcesDir() << std::endl;

    AuthWindow w;
    w.show();
    return a.exec();
}
