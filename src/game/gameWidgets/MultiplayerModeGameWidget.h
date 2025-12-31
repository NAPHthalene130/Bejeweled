#ifndef MULTIPLAYER_MODE_GAME_WIDGET_H
#define MULTIPLAYER_MODE_GAME_WIDGET_H

#include <QWidget>
#include <vector>
#include <string>
#include <QTimer>
#include <QElapsedTimer>
#include <QString>
#include <QVBoxLayout>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DInput/QInputAspect>
#include <map>

class QTextEdit;
class QLabel;
class QPushButton;
class QShowEvent;
class QEvent;
class QHideEvent;

class Gemstone;
class SelectedCircle;
class GameWindow;
class GameNetData;

class MultiplayerModeGameWidget : public QWidget {
    Q_OBJECT
signals:
    void userActionOccurred();

public:
    explicit MultiplayerModeGameWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr, const std::string& userId = "");
    ~MultiplayerModeGameWidget();

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
    void startGame();

    void eliminate();
    void drop();
    void resetGemstoneTable();
    void eliminateAnime(Gemstone* gemstone);
    void switchGemstoneAnime(Gemstone* gemstone1, Gemstone* gemstone2);

    // 消除相关的辅助方法
    std::vector<std::pair<int, int>> findMatches();
    std::vector<std::pair<int, int>> findPossibleMatches();
    void removeMatches(const std::vector<std::pair<int, int>>& matches);

    void syncGemstonePositions();

    void setDifficulty(int diff);
    int getDifficulty() const;

    // Network-related methods
    void sendNetData(const GameNetData& data);
    void handleReceivedData(const GameNetData& data);

    std::map<std::string, int> getIdToNum() const;
    void setIdToNum(const std::map<std::string, int>& map);
    void refreshTabel(int num, const std::vector<std::vector<int>>& table);

    void accept10(std::map<std::string, int> idToNum);
    void accept2(std::string id, std::vector<std::pair<int, int>> coordinates, std::string score);
    // Public access to player tables for testing/debug if needed, though usually internal
    const std::vector<std::vector<Gemstone*>>& getPlayer1Table() const { return player1Table; }
    const std::vector<std::vector<Gemstone*>>& getPlayer2Table() const { return player2Table; }

    void accept4(std::string id, const std::vector<std::vector<int>>& table);

    void setStop(bool stop);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    bool isStop = false;
    QVector3D getPosition(int row, int col) const;
    void handleGemstoneClicked(Gemstone* gem);
    void handleManualClick(const QPoint& screenPos);
    void appendDebug(const QString& text);
    void refreshDebugStatus();

    void clearHighlights();
    void highlightMatches();
    void resetInactivityTimer();

    void updateScoreBoard();
    void updateTimeBoard();
    void triggerFinishIfNeeded();
    void finishToFinalWidget();

    bool findGemstonePosition(Gemstone* gem, int& row, int& col) const;
    bool areAdjacent(int row1, int col1, int row2, int col2) const;
    void performSwap(Gemstone* gem1, Gemstone* gem2, int row1, int col1, int row2, int col2);

    // Network private methods
    void handleSwapMessage(const GameNetData& data);
    void handleEliminateMessage(const GameNetData& data);
    void handleGenerateMessage(const GameNetData& data);
    void handleSyncMessage(const GameNetData& data);
    void handleConnectivityTest(const GameNetData& data);
    void updateOtherPlayerScore(const std::string& playerId, int score);
    void sendBoardSyncMessage();  // Send type=4 sync message
    std::vector<std::vector<int>> getCurrentBoardState() const;  // Get current board state as int array

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

    // Network members
    std::string myUserId;
    std::map<std::string, int> idToNum;  // Player ID -> Player number mapping
    std::map<int, std::string> numToId;  // Player number -> Player ID mapping
    bool allPlayersReady = false;
    QTimer* syncTimer;  // Timer for periodic board synchronization


    class GameTimeKeeper {
    public:
        void reset();
        void start();
        void pause();
        int totalSeconds() const;
        QString displayText() const;
    private:
        QElapsedTimer timer;
        qint64 accumulatedMs = 0;
        bool isRunning = false;
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
    std::vector<SelectedCircle*> highlightRings;  // 用于标记可消除宝石的高亮环
    
    int selectedNum;

    void setup3DScene();

    // Debug UI
    QTextEdit* debugText;
    QLabel* focusInfoLabel;
    QTimer* debugTimer;

    QWidget* rightPanel = nullptr;
    QLabel* scoreBoardLabel = nullptr;
    QLabel* timeBoardLabel = nullptr;
    QLabel* waitingLabel = nullptr;  // Waiting for other players label
    QLabel* connectionStatusLabel = nullptr;  // Connection status
    QPushButton* backToMenuButton = nullptr;

    // Other players' board display
    QWidget* otherPlayersPanelWidget = nullptr;
    QVBoxLayout* otherPlayersPanelLayout = nullptr;
    
    // Player 1 (Top Small Window)
    Qt3DExtras::Qt3DWindow* player1Window = nullptr;
    QWidget* player1Container = nullptr;
    Qt3DCore::QEntity* player1RootEntity = nullptr;
    Qt3DRender::QCamera* player1Camera = nullptr;
    std::vector<std::vector<Gemstone*>> player1Table;
    QLabel* player1ScoreLabel = nullptr;

    // Player 2 (Bottom Small Window)
    Qt3DExtras::Qt3DWindow* player2Window = nullptr;
    QWidget* player2Container = nullptr;
    Qt3DCore::QEntity* player2RootEntity = nullptr;
    Qt3DRender::QCamera* player2Camera = nullptr;
    std::vector<std::vector<Gemstone*>> player2Table;
    QLabel* player2ScoreLabel = nullptr;

    void setupSmall3DWindow(Qt3DExtras::Qt3DWindow* window, Qt3DCore::QEntity** root, Qt3DRender::QCamera** camera);
    void sendCoordinates(std::vector<std::pair<int, int>> coordinates);
    void sendNowBoard();
};

#endif // MULTIPLAYER_MODE_GAME_WIDGET_H
