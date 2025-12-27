#include "NetDataIO.h"
#include "../GameWindow.h"
#include "../gameWidgets/MenuWidget.h"
#include <iostream>
#include <QMetaObject>
#include <QCoreApplication>
#include <boost/asio.hpp>
#include "../../auth/components/AuthNoticeDialog.h"
#include "../gameWidgets/MultiGameWaitWidget.h"

using boost::asio::ip::tcp;

NetDataIO::NetDataIO(std::string ip, std::string port, GameWindow* gameWindow) 
    : ip(ip), port(port), gameWindow(gameWindow), isRunning(true), socket(io_context) {
    
    try {
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, port);
        boost::asio::connect(socket, endpoints);
    } catch (std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return;
    }

    // Start threads
    dataReader = std::thread(&NetDataIO::readerLoop, this);
    dataSender = std::thread(&NetDataIO::senderLoop, this);
}

NetDataIO::~NetDataIO() {
    isRunning = false;
    queueCv.notify_all();
    
    // Cancel any pending operations and close the socket
    boost::system::error_code ec;
    socket.shutdown(tcp::socket::shutdown_both, ec);
    socket.close(ec);
    
    if (dataReader.joinable()) dataReader.join();
    if (dataSender.joinable()) dataSender.join();
}

void NetDataIO::sendData(GameNetData gameData) {
    nlohmann::json j;
    to_json(j, gameData);
    std::string data = j.dump();
    
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        sendQueue.push(data);
    }
    queueCv.notify_one();
}

void NetDataIO::senderLoop() {
    while (isRunning) {
        std::string data;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCv.wait(lock, [this]{ return !sendQueue.empty() || !isRunning; });
            
            if (!isRunning) break;
            
            if (!sendQueue.empty()) {
                data = sendQueue.front();
                sendQueue.pop();
            }
        }
        
        if (!data.empty()) {
            boost::system::error_code ec;
            boost::asio::write(socket, boost::asio::buffer(data), ec);
            if (ec) {
                std::cerr << "Send failed: " << ec.message() << std::endl;
                // If needed, handle disconnection logic here or let readerLoop handle it
            }
        }
    }
}

void NetDataIO::readerLoop() {
    const int recvbuflen = 4096;
    char recvbuf[recvbuflen];

    while (isRunning) {
        boost::system::error_code ec;
        size_t length = socket.read_some(boost::asio::buffer(recvbuf, recvbuflen - 1), ec);
        
        if (!ec) {
            recvbuf[length] = '\0';
            std::string receivedStr(recvbuf);
            
            try {
                nlohmann::json j = nlohmann::json::parse(receivedStr);
                GameNetData receiveData;
                from_json(j, receiveData);
                
                int type = receiveData.getType();
                if (type == 0) {
                    std::string dataStr = receiveData.getData();
                    if (dataStr == "ENTER_ROOM") {
                         QMetaObject::invokeMethod(gameWindow, [this]() {
                            if (gameWindow->getMultiGameWaitWidget()) {
                                gameWindow->getMultiGameWaitWidget()->enterRoom();
                            }
                         }, Qt::QueuedConnection);
                    } else if (dataStr == "GAME_STARTED") {
                         QMetaObject::invokeMethod(gameWindow, [this]() {
                            AuthNoticeDialog* dialog = new AuthNoticeDialog("提示", "房间已开始，请等待游戏结束", 3, gameWindow);
                            dialog->exec();
                         }, Qt::QueuedConnection);
                    }
                } else if (type == 1) {
                    //TODO
                } else if (type == 2) {
                    //TODO
                } else if (type == 3) {
                    //TODO
                } else if (type == 10) {
                    //TODO
                } else if (type == 11) {
                    //TODO
                } else if (type == 12) {
                    //TODO
                } else if (type == 13) {
                    //TODO
                }
                
            } catch (const std::exception& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
            
        } else {
            if (ec == boost::asio::error::eof) {
                std::cout << "Connection closed by server" << std::endl;
            } else if (ec != boost::asio::error::operation_aborted) {
                std::cerr << "Receive failed: " << ec.message() << std::endl;
            }
            break;
        }
    }
    
    // Connection lost or closed
    if (isRunning && gameWindow) {
        // Switch to MenuWidget on the main thread
        QMetaObject::invokeMethod(gameWindow, [this]() {
            if (gameWindow->getMenuWidget()) {
                gameWindow->switchWidget(gameWindow->getMenuWidget());
            }
        }, Qt::QueuedConnection);
    }
    
    isRunning = false;
    queueCv.notify_all();
}
