#include "AuthServer.h"
#include <iostream>

AuthServer::AuthServer(unsigned short port)
    : acceptor(ioContext, tcp::endpoint(tcp::v4(), port)) {
    // Minimal stub: do not use OpenSSL; just set up acceptor but don't generate keys
    std::cout << "AuthServer (stub) listening on port " << port << " (OpenSSL disabled)" << std::endl;
    startAccept();
}

AuthServer::~AuthServer() {
    stop();
}

void AuthServer::run() {
    std::size_t threadPoolSize = 1;
    workerThreads.reserve(threadPoolSize);
    for (std::size_t i = 0; i < threadPoolSize; ++i) {
        workerThreads.emplace_back([this, i]() {
            try {
                ioContext.run();
            } catch (const std::exception& e) {
                std::cerr << "Exception in worker thread: " << e.what() << std::endl;
            }
        });
    }
    for (auto& t : workerThreads) if (t.joinable()) t.join();
}

void AuthServer::stop() {
    if (!stopped.exchange(true)) {
        boost::system::error_code ec;
        acceptor.close(ec);
        ioContext.stop();
    }
}

void AuthServer::startAccept() {
    if (stopped) return;
    auto newSocket = std::make_shared<tcp::socket>(ioContext);
    acceptor.async_accept(*newSocket, [this, newSocket](const boost::system::error_code& error) {
        handleAccept(newSocket, error);
    });
}

void AuthServer::handleAccept(std::shared_ptr<tcp::socket> socket,
                               const boost::system::error_code& error) {
    if (!error && !stopped) {
        startReceive(socket);
        startAccept();
    }
}

void AuthServer::startReceive(std::shared_ptr<tcp::socket> socket) {
    if (stopped) return;
    auto buffer = std::make_shared<std::vector<char>>(4096);
    socket->async_read_some(boost::asio::buffer(*buffer),
        [this, socket, buffer](const boost::system::error_code& error, std::size_t bytesTransferred) {
            handleReceive(socket, buffer, error, bytesTransferred);
        });
}

void AuthServer::handleReceive(std::shared_ptr<tcp::socket> socket,
                               std::shared_ptr<std::vector<char>> buffer,
                               const boost::system::error_code& error,
                               std::size_t bytesTransferred) {
    if (stopped) return;
    if (!error) {
        std::string received(buffer->data(), bytesTransferred);
        std::string response = "STUB:" + received;
        boost::asio::async_write(*socket, boost::asio::buffer(response), [socket](const boost::system::error_code& /*err*/, std::size_t){ });
        startReceive(socket);
    }
}

bool AuthServer::emailCodeSend(std::string) { return false; }
void AuthServer::generateKeys() { /* stub */ }
std::string AuthServer::rsaDecryptBase64(const std::string&) { return std::string(); }
