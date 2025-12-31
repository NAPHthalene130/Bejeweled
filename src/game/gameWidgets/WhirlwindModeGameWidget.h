#ifndef WHIRLWIND_MODE_GAME_WIDGET_H
#define WHIRLWIND_MODE_GAME_WIDGET_H

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

class QTextEdit;
class QLabel;
class QPushButton;
class QProgressBar;
class QShowEvent;
class QEvent;
class QHideEvent;

class Gemstone;
class RotationSquare;
class GameWindow;

class WhirlwindModeGameWidget : public QWidget {
    Q_OBJECT
signals:
    void userActionOccurred();

public:
    explicit WhirlwindModeGameWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    ~WhirlwindModeGameWidget();

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
    void rotateGemstonesAnime(Gemstone* topLeft, Gemstone* topRight,
                               Gemstone* bottomRight, Gemstone* bottomLeft);

    // 消除相关的辅助方法
    std::vector<std::pair<int, int>> findMatches();
    std::vector<std::pair<int, int>> findPossibleMatches();
    void removeMatches(const std::vector<std::pair<int, int>>& matches);

    void syncGemstonePositions();

    void setDifficulty(int diff);
    int getDifficulty() const;

    // 金币系统相关
    void generateCoinGems(int count);
    void collectCoinGem(Gemstone* gem);
    int getEarnedCoins() const; // 获取本局获得的金币数

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QVector3D getPosition(int row, int col) const;
    void handleGemstoneClicked(Gemstone* gem);
    void handleManualClick(const QPoint& screenPos);
    void handleMouseMove(const QPoint& screenPos);
    void appendDebug(const QString& text);
    void refreshDebugStatus();

    void resetNoEliminationTimer();
    void handleNoElimination();
    void updateNoEliminationProgress();

    void updateScoreBoard();
    void updateTimeBoard();
    void triggerFinishIfNeeded();
    void finishToFinalWidget();

    // 辅助函数：找到宝石在容器中的位置
    bool findGemstonePosition(Gemstone* gem, int& row, int& col) const;
    // 检查位置是否可以作为2x2方框的左上角
    bool canFormSquare(int row, int col) const;
    // 执行2x2顺时针旋转
    void performRotation(int topLeftRow, int topLeftCol);

    Qt3DExtras::Qt3DWindow* game3dWindow;
    QWidget* container3d;
    std::vector<std::vector<Gemstone*>> gemstoneContainer;
    std::string style;
    bool canOpe;
    QTimer* timer;
    int nowTimeHave;
    int mode; // 2: Whirlwind/Rotate mode
    int comboCount = 0;

    int difficulty = 4;

    GameWindow* gameWindow;

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

    // Selection State for Whirlwind Mode
    int selectedTopLeftRow;
    int selectedTopLeftCol;
    bool hasSelection;

    RotationSquare* rotationSquare;

    // 鼠标悬停显示的旋转框
    RotationSquare* hoverSquare;
    int hoverRow = -1;
    int hoverCol = -1;

    // 十秒未消除则结束的计时器
    QTimer* noEliminationTimer;
    int noEliminationTimeout = 10000; // 10秒
    int noEliminationTimeRemaining = 10000; // 剩余时间(ms)

    void setup3DScene();

    // Debug UI
    QTextEdit* debugText;
    QLabel* focusInfoLabel;
    QTimer* debugTimer;

    QWidget* rightPanel = nullptr;
    QLabel* scoreBoardLabel = nullptr;
    QLabel* timeBoardLabel = nullptr;
    QPushButton* backToMenuButton = nullptr;

    // 时间追逐进度条
    QProgressBar* noEliminationProgressBar = nullptr;

    // 金币统计
    int initialCoins = 0;  // 游戏开始时的金币数
    int earnedCoins = 0;   // 本局获得的金币数
};

#endif // WHIRLWIND_MODE_GAME_WIDGET_H
