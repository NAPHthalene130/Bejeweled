#ifndef MULTIPLAYER_MODE_GAME_WIDGET_H
#define MULTIPLAYER_MODE_GAME_WIDGET_H

#include <QWidget>
#include <vector>
#include <string>
#include <QTimer>
#include <QString>
#include <QTcpSocket>
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
    void connectToServer(const QString& host, int port);
    void sendNetData(const GameNetData& data);
    void handleReceivedData(const GameNetData& data);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
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
    void sendEnterRoomMessage();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    void handleSwapMessage(const GameNetData& data);
    void handleEliminateMessage(const GameNetData& data);
    void handleGenerateMessage(const GameNetData& data);
    void handleSyncMessage(const GameNetData& data);
    void handleConnectivityTest(const GameNetData& data);
    void updateOtherPlayerBoard(const std::string& playerId, const std::vector<std::vector<int>>& board);
    void updateOtherPlayerScore(const std::string& playerId, int score);
    void sendBoardSyncMessage();  // Send type=4 sync message
    std::vector<std::vector<int>> getCurrentBoardState() const;  // Get current board state as int array

    // UI methods for other players' boards
    void createOtherPlayerBoardWidget(const std::string& playerId);
    void updateOtherPlayerBoardUI(const std::string& playerId);

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
    QTcpSocket* socket;
    std::string myUserId;
    std::map<std::string, int> idToNum;  // Player ID -> Player number mapping
    std::map<int, std::string> numToId;  // Player number -> Player ID mapping
    std::map<std::string, std::vector<std::vector<int>>> otherPlayersBoards;  // Other players' board states (as int types)
    std::map<std::string, int> otherPlayersScores;  // Other players' scores
    bool isConnected = false;
    bool allPlayersReady = false;
    QByteArray receiveBuffer;  // Buffer for incomplete messages
    QTimer* syncTimer;  // Timer for periodic board synchronization

    class GameTimeKeeper {
    public:
        void reset();
        void tick();
        int totalSeconds() const;
        QString displayText() const;
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
    std::map<std::string, QLabel*> otherPlayersLabels;  // Player ID -> Score/Info Label
    std::map<std::string, QWidget*> otherPlayersBoardWidgets;  // Player ID -> Board Widget
    std::map<std::string, std::vector<std::vector<QLabel*>>> otherPlayersBoardCells;  // Player ID -> Board cells
};

#endif // MULTIPLAYER_MODE_GAME_WIDGET_H
