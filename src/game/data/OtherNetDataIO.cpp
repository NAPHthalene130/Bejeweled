#include "OtherNetDataIO.h"
#include "OtherNetData.h"
#include "../GameWindow.h"
#include <boost/asio.hpp>
#include <iostream>

OtherNetDataIO::OtherNetDataIO(GameWindow* gameWindow) {
    this->gameWindow = gameWindow;
}

OtherNetDataIO::~OtherNetDataIO() {
    gameWindow = nullptr;
}

bool OtherNetDataIO::setMoney(std::string id, int money) {
    if (!gameWindow) return false;

    OtherNetData otherNetData;
    otherNetData.setType(21); // Corrected to Type 21
    otherNetData.setId(id);
    otherNetData.setMoney(money);
    std::string ip = gameWindow->getIp();
    int port = 10088;
    
    bool success = false;

    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::steady_timer timer(io_context);

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) {
                socket.close();
            }
        });

        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, std::to_string(port));

        nlohmann::json j;
        to_json(j, otherNetData);
        std::string jsonStr = j.dump();

        boost::asio::async_connect(socket, endpoints,
            [&](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                if (!ec) {
                    boost::asio::async_write(socket, boost::asio::buffer(jsonStr),
                        [&](const boost::system::error_code& ec, std::size_t) {
                            if (!ec) {
                                success = true; 
                                timer.cancel();
                            }
                        });
                }
            });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "OtherNetDataIO::setMoney failed: " << e.what() << std::endl;
        return false;
    }
    
    return success;
}

int OtherNetDataIO::getMoney(std::string id) {
    if (!gameWindow) return 0;

    OtherNetData dataRequest;
    dataRequest.setType(20); // Corrected to Type 20
    dataRequest.setId(id);
    std::string ip = gameWindow->getIp();
    int port = 10088;

    int resultMoney = 0;
    bool success = false;

    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::steady_timer timer(io_context);

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) {
                socket.close();
            }
        });

        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, std::to_string(port));

        nlohmann::json j;
        to_json(j, dataRequest);
        std::string jsonStr = j.dump();

        auto responseBuffer = std::make_shared<boost::asio::streambuf>();

        boost::asio::async_connect(socket, endpoints,
            [&](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                if (!ec) {
                    boost::asio::async_write(socket, boost::asio::buffer(jsonStr),
                        [&](const boost::system::error_code& ec, std::size_t) {
                            if (!ec) {
                                boost::asio::async_read(socket, *responseBuffer, boost::asio::transfer_at_least(1),
                                    [&, responseBuffer](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                        if (!ec || ec == boost::asio::error::eof) {
                                            try {
                                                std::string responseStr((std::istreambuf_iterator<char>(responseBuffer.get())), std::istreambuf_iterator<char>());
                                                if (!responseStr.empty()) {
                                                    nlohmann::json respJ = nlohmann::json::parse(responseStr);
                                                    OtherNetData responseData;
                                                    from_json(respJ, responseData);
                                                    resultMoney = responseData.getMoney();
                                                    success = true;
                                                }
                                            } catch (...) {}
                                            timer.cancel();
                                        }
                                    });
                            }
                        });
                }
            });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "OtherNetDataIO::getMoney failed: " << e.what() << std::endl;
        return 0;
    }

    return success ? resultMoney : 0;
}

bool OtherNetDataIO::setAchievementStr(std::string id, std::string achievementStr) {
    if (!gameWindow) return false;

    OtherNetData otherNetData;
    otherNetData.setType(11);
    otherNetData.setId(id);
    otherNetData.setAchievementStr(achievementStr);
    std::string ip = gameWindow->getIp();
    int port = 10088;
    
    bool success = false;

    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::steady_timer timer(io_context);

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) socket.close();
        });

        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, std::to_string(port));

        nlohmann::json j;
        to_json(j, otherNetData);
        std::string jsonStr = j.dump();

        boost::asio::async_connect(socket, endpoints,
            [&](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                if (!ec) {
                    boost::asio::async_write(socket, boost::asio::buffer(jsonStr),
                        [&](const boost::system::error_code& ec, std::size_t) {
                            if (!ec) {
                                success = true;
                                timer.cancel();
                            }
                        });
                }
            });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "OtherNetDataIO::setAchievementStr failed: " << e.what() << std::endl;
        return false;
    }
    return success;
}

std::string OtherNetDataIO::getAchievementStr(std::string id) {
    if (!gameWindow) return "";

    OtherNetData dataRequest;
    dataRequest.setType(10);
    dataRequest.setId(id);
    std::string ip = gameWindow->getIp();
    int port = 10088;

    std::string resultStr = "";
    bool success = false;

    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::steady_timer timer(io_context);

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) socket.close();
        });

        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, std::to_string(port));

        nlohmann::json j;
        to_json(j, dataRequest);
        std::string jsonStr = j.dump();

        auto responseBuffer = std::make_shared<boost::asio::streambuf>();

        boost::asio::async_connect(socket, endpoints,
            [&](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                if (!ec) {
                    boost::asio::async_write(socket, boost::asio::buffer(jsonStr),
                        [&](const boost::system::error_code& ec, std::size_t) {
                            if (!ec) {
                                boost::asio::async_read(socket, *responseBuffer, boost::asio::transfer_at_least(1),
                                    [&, responseBuffer](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                        if (!ec || ec == boost::asio::error::eof) {
                                            try {
                                                std::string responseStr((std::istreambuf_iterator<char>(responseBuffer.get())), std::istreambuf_iterator<char>());
                                                if (!responseStr.empty()) {
                                                    nlohmann::json respJ = nlohmann::json::parse(responseStr);
                                                    OtherNetData responseData;
                                                    from_json(respJ, responseData);
                                                    resultStr = responseData.getAchievementStr();
                                                    success = true;
                                                }
                                            } catch (...) {}
                                            timer.cancel();
                                        }
                                    });
                            }
                        });
                }
            });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "OtherNetDataIO::getAchievementStr failed: " << e.what() << std::endl;
        return "";
    }
    return success ? resultStr : "";
}

std::vector<std::vector<std::pair<std::string, int>>> OtherNetDataIO::getRanks() {
    std::vector<std::vector<std::pair<std::string, int>>> ranks;
    if (!gameWindow) return ranks;

    OtherNetData dataRequest;
    dataRequest.setType(30);
    std::string ip = gameWindow->getIp();
    int port = 10088;

    bool success = false;

    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::steady_timer timer(io_context);

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) socket.close();
        });

        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, std::to_string(port));

        nlohmann::json j;
        to_json(j, dataRequest);
        std::string jsonStr = j.dump();

        auto responseBuffer = std::make_shared<boost::asio::streambuf>();

        boost::asio::async_connect(socket, endpoints,
            [&](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                if (!ec) {
                    boost::asio::async_write(socket, boost::asio::buffer(jsonStr),
                        [&](const boost::system::error_code& ec, std::size_t) {
                            if (!ec) {
                                boost::asio::async_read(socket, *responseBuffer, boost::asio::transfer_at_least(1),
                                    [&, responseBuffer](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                        if (!ec || ec == boost::asio::error::eof) {
                                            try {
                                                std::string responseStr((std::istreambuf_iterator<char>(responseBuffer.get())), std::istreambuf_iterator<char>());
                                                if (!responseStr.empty()) {
                                                    nlohmann::json respJ = nlohmann::json::parse(responseStr);
                                                    OtherNetData responseData;
                                                    from_json(respJ, responseData);
                                                    
                                                    ranks.push_back(responseData.getNormalRank());
                                                    ranks.push_back(responseData.getWhirlRank());
                                                    ranks.push_back(responseData.getMultiRank());
                                                    success = true;
                                                }
                                            } catch (...) {}
                                            timer.cancel();
                                        }
                                    });
                            }
                        });
                }
            });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "OtherNetDataIO::getRanks failed: " << e.what() << std::endl;
        return std::vector<std::vector<std::pair<std::string, int>>>();
    }
    return success ? ranks : std::vector<std::vector<std::pair<std::string, int>>>();
}

bool OtherNetDataIO::setPropNums(std::string id, std::vector<int> propNums) {
    if (!gameWindow) return false;

    OtherNetData otherNetData;
    otherNetData.setType(41);
    otherNetData.setId(id);
    otherNetData.setPropNums(propNums);
    std::string ip = gameWindow->getIp();
    int port = 10088;
    
    bool success = false;

    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::steady_timer timer(io_context);

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) socket.close();
        });

        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, std::to_string(port));

        nlohmann::json j;
        to_json(j, otherNetData);
        std::string jsonStr = j.dump();

        boost::asio::async_connect(socket, endpoints,
            [&](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                if (!ec) {
                    boost::asio::async_write(socket, boost::asio::buffer(jsonStr),
                        [&](const boost::system::error_code& ec, std::size_t) {
                            if (!ec) {
                                success = true;
                                timer.cancel();
                            }
                        });
                }
            });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "OtherNetDataIO::setPropNums failed: " << e.what() << std::endl;
        return false;
    }
    return success;
}

std::vector<int> OtherNetDataIO::getPropNums(std::string id) {
    if (!gameWindow) return std::vector<int>();

    OtherNetData dataRequest;
    dataRequest.setType(40);
    dataRequest.setId(id);
    std::string ip = gameWindow->getIp();
    int port = 10088;

    std::vector<int> resultProps;
    bool success = false;

    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::steady_timer timer(io_context);

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) socket.close();
        });

        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, std::to_string(port));

        nlohmann::json j;
        to_json(j, dataRequest);
        std::string jsonStr = j.dump();

        auto responseBuffer = std::make_shared<boost::asio::streambuf>();

        boost::asio::async_connect(socket, endpoints,
            [&](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                if (!ec) {
                    boost::asio::async_write(socket, boost::asio::buffer(jsonStr),
                        [&](const boost::system::error_code& ec, std::size_t) {
                            if (!ec) {
                                boost::asio::async_read(socket, *responseBuffer, boost::asio::transfer_at_least(1),
                                    [&, responseBuffer](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                        if (!ec || ec == boost::asio::error::eof) {
                                            try {
                                                std::string responseStr((std::istreambuf_iterator<char>(responseBuffer.get())), std::istreambuf_iterator<char>());
                                                if (!responseStr.empty()) {
                                                    nlohmann::json respJ = nlohmann::json::parse(responseStr);
                                                    OtherNetData responseData;
                                                    from_json(respJ, responseData);
                                                    resultProps = responseData.getPropNums();
                                                    success = true;
                                                }
                                            } catch (...) {}
                                            timer.cancel();
                                        }
                                    });
                            }
                        });
                }
            });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "OtherNetDataIO::getPropNums failed: " << e.what() << std::endl;
        return std::vector<int>();
    }
    return success ? resultProps : std::vector<int>();
}

bool OtherNetDataIO::sendNormalTime(std::string id, int time) {
    if (!gameWindow) return false;

    OtherNetData otherNetData;
    otherNetData.setType(50);
    otherNetData.setId(id);
    otherNetData.setNormalTime(time);
    std::string ip = gameWindow->getIp();
    int port = 10088;
    
    bool success = false;
    
    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::steady_timer timer(io_context);

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) {
                socket.close();
            }
        });

        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, std::to_string(port));

        nlohmann::json j;
        to_json(j, otherNetData);
        std::string jsonStr = j.dump();

        boost::asio::async_connect(socket, endpoints,
            [&](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                if (!ec) {
                    boost::asio::async_write(socket, boost::asio::buffer(jsonStr),
                        [&](const boost::system::error_code& ec, std::size_t) {
                            if (!ec) {
                                success = true;
                                timer.cancel();
                            }
                        });
                }
            });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "OtherNetDataIO::sendNormalTime failed: " << e.what() << std::endl;
        return false;
    }
    
    return success;
}

bool OtherNetDataIO::sendWhirlTime(std::string id, int time) {
    if (!gameWindow) return false;

    OtherNetData otherNetData;
    otherNetData.setType(51);
    otherNetData.setId(id);
    otherNetData.setWhirlTime(time);
    std::string ip = gameWindow->getIp();
    int port = 10088;

    bool success = false;

    try {
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket(io_context);
        boost::asio::steady_timer timer(io_context);

        timer.expires_after(std::chrono::milliseconds(500));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (!ec) {
                socket.close();
            }
        });

        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(ip, std::to_string(port));

        nlohmann::json j;
        to_json(j, otherNetData);
        std::string jsonStr = j.dump();

        boost::asio::async_connect(socket, endpoints,
            [&](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                if (!ec) {
                    boost::asio::async_write(socket, boost::asio::buffer(jsonStr),
                        [&](const boost::system::error_code& ec, std::size_t) {
                            if (!ec) {
                                success = true;
                                timer.cancel();
                            }
                        });
                }
            });

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "OtherNetDataIO::sendWhirlTime failed: " << e.what() << std::endl;
        return false;
    }
    
    return success;
}
