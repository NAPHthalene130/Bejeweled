#ifndef OTHERNETDATAIO_H
#define OTHERNETDATAIO_H

#include <string>
#include <vector>
#include "OtherNetData.h"


class GameWindow;

class OtherNetDataIO {
private:
    GameWindow* gameWindow = nullptr;;
public:
    OtherNetDataIO(GameWindow* gameWindow);
    ~OtherNetDataIO();

    bool setMoney(std::string id, int money);
    int getMoney(std::string id);

    bool setAchievementStr(std::string id, std::string achievementStr);
    std::string getAchievementStr(std::string id);

    std::vector<std::vector<std::pair<std::string, int>>> getRanks();

    bool setPropNums(std::string id, std::vector<int> propNums);
    std::vector<int> getPropNums(std::string id);

    bool sendNormalTime(std::string id, int time);

    bool sendWhirlTime(std::string id, int time);
};

#endif // OTHERNETDATAIO_H
