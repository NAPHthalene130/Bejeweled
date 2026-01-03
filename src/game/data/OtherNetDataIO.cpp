#include "OtherNetDataIO.h"
#include "OtherNetData.h"
#include "../GameWindow.h"
#include "../../Config.h"
#include <boost/asio.hpp>
#include <iostream>

OtherNetDataIO::OtherNetDataIO(GameWindow* gameWindow) {
    this->gameWindow = gameWindow;
}

OtherNetDataIO::~OtherNetDataIO() {
    gameWindow = nullptr;
}

bool OtherNetDataIO::sendRequest(const OtherNetData& request, OtherNetData* response) {
    std::string ip = Config::getServerIp();
    int port = Config::getOtherNetDataPort();
    bool success = false;

    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::steady_timer timer(io_context);
        boost::asio::ip::tcp::resolver resolver(io_context);

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) {
                socket.close();
                resolver.cancel();
            }
        });

        nlohmann::json j;
        to_json(j, request);
        std::string jsonStr = j.dump();
        auto responseBuffer = std::make_shared<boost::asio::streambuf>();

        resolver.async_resolve(boost::asio::ip::tcp::v4(), ip, std::to_string(port),
            [&](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results) {
                if (!ec) {
                    boost::asio::async_connect(socket, results,
                        [&, response](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                            if (!ec) {
                                boost::asio::async_write(socket, boost::asio::buffer(jsonStr),
                                    [&, response](const boost::system::error_code& ec, std::size_t) {
                                        if (!ec) {
                                            if (response) {
                                                boost::asio::async_read(socket, *responseBuffer, boost::asio::transfer_at_least(1),
                                                    [&, response](const boost::system::error_code& ec, std::size_t) {
                                                        if (!ec || ec == boost::asio::error::eof) {
                                                            try {
                                                                std::string responseStr((std::istreambuf_iterator<char>(responseBuffer.get())), std::istreambuf_iterator<char>());
                                                                if (!responseStr.empty()) {
                                                                    nlohmann::json respJ = nlohmann::json::parse(responseStr);
                                                                    from_json(respJ, *response);
                                                                    success = true;
                                                                }
                                                            } catch (...) {}
                                                            timer.cancel();
                                                        }
                                                    });
                                            } else {
                                                success = true;
                                                timer.cancel();
                                            }
                                        }
                                    });
                            }
                        });
                }
            });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "OtherNetDataIO request failed: " << e.what() << std::endl;
        return false;
    }
    return success;
}

bool OtherNetDataIO::setMoney(std::string id, int money) {
    if (!gameWindow) return false;
    OtherNetData request;
    request.setType(21);
    request.setId(id);
    request.setMoney(money);
    return sendRequest(request);
}

int OtherNetDataIO::getMoney(std::string id) {
    if (!gameWindow) return 0;
    OtherNetData request;
    request.setType(20);
    request.setId(id);
    OtherNetData response;
    if (sendRequest(request, &response)) {
        return response.getMoney();
    }
    return 0;
}

bool OtherNetDataIO::setAchievementStr(std::string id, std::string achievementStr) {
    if (!gameWindow) return false;
    OtherNetData request;
    request.setType(11);
    request.setId(id);
    request.setAchievementStr(achievementStr);
    return sendRequest(request);
}

std::string OtherNetDataIO::getAchievementStr(std::string id) {
    if (!gameWindow) return "";
    OtherNetData request;
    request.setType(10);
    request.setId(id);
    OtherNetData response;
    if (sendRequest(request, &response)) {
        return response.getAchievementStr();
    }
    return "0000000000";
}

std::vector<std::vector<std::pair<std::string, int>>> OtherNetDataIO::getRanks() {
    if (!gameWindow) return {};
    OtherNetData request;
    request.setType(30);
    OtherNetData response;
    if (sendRequest(request, &response)) {
        std::vector<std::vector<std::pair<std::string, int>>> ranks;
        ranks.push_back(response.getNormalRank());
        ranks.push_back(response.getWhirlRank());
        ranks.push_back(response.getMultiRank());
        return ranks;
    }
    return {{},{},{}};
}

bool OtherNetDataIO::setPropNums(std::string id, std::vector<int> propNums) {
    if (!gameWindow) return false;
    OtherNetData request;
    request.setType(41);
    request.setId(id);
    request.setPropNums(propNums);
    return sendRequest(request);
}

std::vector<int> OtherNetDataIO::getPropNums(std::string id) {
    if (!gameWindow) return {};
    OtherNetData request;
    request.setType(40);
    request.setId(id);
    OtherNetData response;
    if (sendRequest(request, &response)) {
        return response.getPropNums();
    }
    return {0,0,0,0};
}

bool OtherNetDataIO::sendNormalTime(std::string id, int time) {
    if (!gameWindow) return false;
    OtherNetData request;
    request.setType(50);
    request.setId(id);
    request.setNormalTime(time);
    return sendRequest(request);
}

bool OtherNetDataIO::sendWhirlTime(std::string id, int time) {
    if (!gameWindow) return false;
    OtherNetData request;
    request.setType(51);
    request.setId(id);
    request.setWhirlTime(time);
    return sendRequest(request);
}
