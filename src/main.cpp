#include "Auth/AuthWindow.h"
#include <QApplication>
#include <QTimer>
#include <iostream>
#include "utils/ResourceUtils.h"
#include "Game/GameWindow.h"
#include "Game/gameWidgets/SingleModeGameWidget.h"

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

    // 直接启动GameWindow并进入单人游戏模式（用于测试）
    GameWindow* mainUI = new GameWindow();
    mainUI->show();

    // 自动进入单人游戏模式
    QTimer::singleShot(500, [mainUI]() {
        auto* singleMode = mainUI->getSingleModeGameWidget();
        if (singleMode) {
            singleMode->reset(1); // 普通模式
            mainUI->switchWidget(singleMode);
        }
    });

    // AuthWindow w;
    // w.show();
    return a.exec();
}
