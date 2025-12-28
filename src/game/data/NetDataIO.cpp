#include "NetDataIO.h"
#include "../GameWindow.h"
#include "../gameWidgets/MenuWidget.h"
#include "../../utils/LogWindow.h"
#include <iostream>
#include <QMetaObject>
#include <QCoreApplication>
#include <boost/asio.hpp>
#include "../../auth/components/AuthNoticeDialog.h"
#include "../gameWidgets/MultiGameWaitWidget.h"

using boost::asio::ip::tcp;

NetDataIO::NetDataIO(std::string ip, std::string port, GameWindow* gameWindow) 
    : ip(ip), port(port), gameWindow(gameWindow), isRunning(true), socket(io_context) {
    
    log("INFO", "NetDataIO", "Connecting to " + ip + ":" + port);
    try {
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, port);
        boost::asio::connect(socket, endpoints);
        log("INFO", "NetDataIO", "Connected successfully");
    } catch (std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        log("ERROR", "NetDataIO", "Connection failed: " + std::string(e.what()));
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
    
    log("INFO", "sendData", "Sending data: " + data);

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
                log("ERROR", "senderLoop", "Send failed: " + ec.message());
                // If needed, handle disconnection logic here or let readerLoop handle it
            }
        }
    }
}

void NetDataIO::readerLoop() {
    const int recvbuflen = 4096;
    char recvbuf[recvbuflen];
    std::string accumulatedBuffer;

    while (isRunning) {
        boost::system::error_code ec;
        size_t length = socket.read_some(boost::asio::buffer(recvbuf, recvbuflen - 1), ec);
        
        if (!ec) {
            recvbuf[length] = '\0';
            std::string receivedStr(recvbuf);
            std::cout << "Received raw: " << receivedStr << std::endl;
            // log("INFO", "readerLoop", "Received raw: " + receivedStr);

            accumulatedBuffer += receivedStr;
            
            // Process accumulated buffer for complete JSON objects
            while (!accumulatedBuffer.empty()) {
                // Skip whitespace/garbage at start
                size_t startPos = accumulatedBuffer.find('{');
                if (startPos == std::string::npos) {
                    // No JSON start found, clear buffer to avoid memory growth if it's just garbage
                    if (accumulatedBuffer.length() > 4096) accumulatedBuffer.clear(); 
                    break; 
                }
                
                if (startPos > 0) {
                    accumulatedBuffer.erase(0, startPos);
                    startPos = 0;
                }

                // Try to find the matching closing brace
                int braceCount = 0;
                bool inString = false;
                bool escaped = false;
                size_t endPos = std::string::npos;
                
                for (size_t i = 0; i < accumulatedBuffer.length(); ++i) {
                    char c = accumulatedBuffer[i];
                    if (escaped) {
                        escaped = false;
                        continue;
                    }
                    if (c == '\\') {
                        escaped = true;
                        continue;
                    }
                    if (c == '"') {
                        inString = !inString;
                        continue;
                    }
                    
                    if (!inString) {
                        if (c == '{') {
                            braceCount++;
                        } else if (c == '}') {
                            braceCount--;
                            if (braceCount == 0) {
                                endPos = i;
                                break;
                            }
                        }
                    }
                }

                if (endPos != std::string::npos) {
                    // Found a complete JSON object
                    std::string jsonStr = accumulatedBuffer.substr(0, endPos + 1);
                    accumulatedBuffer.erase(0, endPos + 1);
                    
                    log("INFO", "readerLoop", "Processing JSON: " + jsonStr);
                    
                    try {
                        nlohmann::json j = nlohmann::json::parse(jsonStr);
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
                            int roomPeopleHave = std::stoi(receiveData.getData());
                            QMetaObject::invokeMethod(gameWindow, [this, roomPeopleHave]() {
                                if (gameWindow->getMultiGameWaitWidget()) {
                                    gameWindow->getMultiGameWaitWidget()->setRoomPeopleHave(roomPeopleHave);
                                }
                            }, Qt::QueuedConnection);
                        } else if (type == 12) {
                            //TODO
                        } else if (type == 13) {
                            //TODO
                        }
                        
                    } catch (const std::exception& e) {
                        std::cerr << "JSON parse error: " << e.what() << std::endl;
                        log("ERROR", "readerLoop", "JSON parse error: " + std::string(e.what()));
                    }
                } else {
                    // Incomplete JSON, wait for more data
                    break;
                }
            }
            
        } else {
            if (ec == boost::asio::error::eof) {
                std::cout << "Connection closed by server" << std::endl;
                log("WARN", "readerLoop", "Connection closed by server");
            } else if (ec != boost::asio::error::operation_aborted) {
                std::cerr << "Receive failed: " << ec.message() << std::endl;
                log("ERROR", "readerLoop", "Receive failed: " + ec.message());
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

void NetDataIO::log(const std::string& nature, const std::string& methodName, const std::string& content) {
    if (gameWindow && gameWindow->getLogWindow()) {
        std::string msg = "[" + nature + "][NetDataIO][" + methodName + "]:" + content;
        // Since this might be called from background threads, use invokeMethod to update UI
        QMetaObject::invokeMethod(gameWindow->getLogWindow(), [this, msg]() {
             if (gameWindow && gameWindow->getLogWindow()) {
                gameWindow->getLogWindow()->logWrite(msg);
             }
        }, Qt::QueuedConnection);
    }
}
