#ifndef SINGLE_MODE_GAME_WIDGET_H
#define SINGLE_MODE_GAME_WIDGET_H

#include <QWidget>
#include <vector>
#include <string>
#include <QTimer>
#include <QString>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DInput/QInputAspect>
#include "../data/ItemSystem.h"
#include <QPropertyAnimation> // 新增

class QTextEdit;
class QLabel;
class QPushButton;
class QShowEvent;
class QEvent;
class QHideEvent;

class Gemstone;
class SelectedCircle;
class GameWindow;
class ScoreProgressBar;

class SingleModeGameWidget : public QWidget {
    Q_OBJECT
signals:
    void userActionOccurred();

public:
    explicit SingleModeGameWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    ~SingleModeGameWidget();

    Qt3DExtras::Qt3DWindow* getGame3dWindow() const;
    
    std::vector<std::vector<Gemstone*>> getGemstoneContainer() const;
    void setGemstoneContainer(const std::vector<std::vector<Gemstone*>>& container);

    std::string getStyle() const;
    void setStyle(const std::string& style);

    bool getCanOpe() const;
    void setCanOpe(bool canOpe);

    QTimer* getTimer() const;
    void setTimer(QTimer* timer);

    int getNowTimeHave() const;
    void setNowTimeHave(int time);

    int getMode() const;
    void setMode(int mode);
    void reset(int mode);

    void eliminate();
    void drop();
    void resetGemstoneTable();
    void eliminateAnime(Gemstone* gemstone);
    void switchGemstoneAnime(Gemstone* gemstone1, Gemstone* gemstone2);

    // 消除相关的辅助方法
    std::vector<std::pair<int, int>> findMatches(int x,int y,int T);
    std::vector<std::pair<int, int>> findPossibleMatches();
    void removeMatches(const std::vector<std::pair<int, int>>& matches);

    void syncGemstonePositions();

    void setDifficulty(int diff);
    int getDifficulty() const;

    // 金币系统相关
    void generateCoinGems(int count);
    void collectCoinGem(Gemstone* gem);
    int getEarnedCoins() const; // 获取本局获得的金币数

    // 道具系统相关
    void useItemFreezeTime();
    void useItemHammer();
    void useItemResetBoard();
    void useItemClearAll();
    void enableHammerMode();
    void disableHammerMode();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QVector3D getPosition(int row, int col) const;
    void handleGemstoneClicked(Gemstone* gem);
    void handleManualClick(const QPoint& screenPos , int kind); // 手动处理点击
    void appendDebug(const QString& text);
    void refreshDebugStatus();

    void clearHighlights();
    void highlightMatches();
    void showFloatingMessage(const QString& text, bool isSuccess);
    void removeFloatingMessage(QLabel* label);

    void resetInactivityTimer();

    void updateScoreBoard();
    void updateTimeBoard();
    void updateItemButtons();  // 更新道具按钮的启用/禁用状态
    void triggerFinishIfNeeded();
    void finishToFinalWidget();

    // 辅助函数：找到宝石在容器中的位置
    bool findGemstonePosition(Gemstone* gem, int& row, int& col) const;
    // 检查两个位置是否相邻
    bool areAdjacent(int row1, int col1, int row2, int col2) const;
    // 执行交换
    void performSwap(Gemstone* gem1, Gemstone* gem2, int row1, int col1, int row2, int col2);

    // 将匹配的宝石分组（识别连续的4连或更多）
    std::vector<std::vector<std::pair<int, int>>> groupMatches(
        const std::vector<std::pair<int, int>>& matches);
    
    // 消除以特殊宝石为中心的3×3区域
    void remove3x3Area(int centerRow, int centerCol);
    
    // 检查匹配组中是否包含特殊宝石
    bool hasSpecialGem(const std::vector<std::pair<int, int>>& group) const;

    void remove3x3AreaChain(int centerRow, int centerCol);

    Qt3DExtras::Qt3DWindow* game3dWindow;
    QWidget* container3d;
    std::vector<std::vector<Gemstone*>> gemstoneContainer;
    std::string style;
    bool canOpe;
    QTimer* timer;
    int nowTimeHave;
    int mode; // 1: Normal, 2: Rotate
    int comboCount = 0; 
    
    int difficulty = 4;

    GameWindow* gameWindow;

    class GameTimeKeeper {
    public:
        void reset();
        void tick();
        int totalSeconds() const;
        QString displayText() const;
        void setSeconds(int s) { seconds = s; }
    private:
        int seconds = 0;
    };

    GameTimeKeeper gameTimeKeeper;
    int gameScore = 0;
    int targetScore = 100;
    bool isFinishing = false;

    // Qt3D Members
    Qt3DCore::QEntity* rootEntity;
    Qt3DRender::QCamera* cameraEntity;
    Qt3DCore::QEntity* lightEntity;

    // Selection State
    Gemstone* firstSelectedGemstone;
    Gemstone* secondSelectedGemstone;
    
    SelectedCircle* selectionRing1;
    SelectedCircle* selectionRing2;

    QTimer* inactivityTimer;       // 无操作计时器
    int inactivityTimeout = 5000;  // 超时时间(毫秒)，这里设为5秒
    std::vector<Gemstone*> highlightGems;  // 用于标记可消除宝石的高亮环
    
    const int dx[4] = {0,0,1,-1};
    const int dy[4] = {1,-1,0,0};
    
    int selectedNum;

    void setup3DScene();

    // Debug UI
    QTextEdit* debugText;
    QLabel* focusInfoLabel;
    QTimer* debugTimer;

    QWidget* rightPanel = nullptr;
    QLabel* scoreBoardLabel = nullptr;
    QLabel* timeBoardLabel = nullptr;
    QPushButton* backToMenuButton = nullptr;
    ScoreProgressBar* scoreProgressBar = nullptr;

    // 道具相关UI
    QWidget* itemPanel = nullptr;
    std::map<ItemType, QPushButton*> itemButtons;
    std::map<ItemType, QLabel*> itemCountLabels;

    // 道具状态
    bool hammerMode = false;
    int freezeTimeRemaining = 0;
    QTimer* freezeTimer = nullptr;
    SelectedCircle* hammerHoverRing = nullptr;  // 锤子模式下的悬停高亮圈
    Gemstone* hammerHoverGem = nullptr;         // 当前鼠标悬停的宝石

    // 金币统计
    int initialCoins = 0;  // 游戏开始时的金币数
    int earnedCoins = 0;   // 本局获得的金币数
    
    //有关滑动交换
    bool isDragging = false;
    bool isClear = false;
};

#endif // SINGLE_MODE_GAME_WIDGET_H
