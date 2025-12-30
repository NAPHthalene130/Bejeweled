#ifndef NET_DATA_IO_H
#define NET_DATA_IO_H

#include <boost/asio.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include "GameNetData.h"

class GameWindow;

class NetDataIO {
public:
    NetDataIO(std::string ip, std::string port, GameWindow* gameWindow);
    ~NetDataIO();

    void sendData(GameNetData gameData);

private:
    std::string ip;
    std::string port;
    GameWindow* gameWindow;

    std::thread dataReader;
    std::thread dataSender;
    
    std::queue<std::string> sendQueue;
    std::mutex queueMutex;
    std::condition_variable queueCv;
    
    std::atomic<bool> isRunning;
    
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;

    void readerLoop();
    void senderLoop();
    void log(const std::string& nature, const std::string& methodName, const std::string& content);
};

#endif // NET_DATA_IO_H
