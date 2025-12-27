#include "Auth/AuthWindow.h"
#include <QApplication>
#include <QTimer>
#include <iostream>
#include "utils/ResourceUtils.h"
#include "Game/GameWindow.h"
#include "Game/gameWidgets/SingleModeGameWidget.h"
#include "game/TestWindow.h"
#include <QRandomGenerator>

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

    // 启动登录注册窗口
    // AuthWindow w;
    // w.show();

    //测试用
    int randNum = QRandomGenerator::global()->bounded(1000000); //随机ID
    GameWindow gameWindow(nullptr, std::to_string(randNum));
    gameWindow.show();

    TestWindow testWindow;
    testWindow.show();
    return a.exec();
}
