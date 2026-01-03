#ifndef PUZZLE_MODE_GAME_WIDGET_H
#define PUZZLE_MODE_GAME_WIDGET_H

#include <QWidget>
#include <vector>
#include <stack>
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
class QShowEvent;
class QEvent;
class QHideEvent;

class Gemstone;
class SelectedCircle;
class GameWindow;
class ScoreProgressBar;
class GradientLevelLabel; 

class PuzzleModeGameWidget : public QWidget {
    Q_OBJECT
signals:
    void userActionOccurred();

public:
    explicit PuzzleModeGameWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    ~PuzzleModeGameWidget();

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
    int findPossibleMatches();
    void removeMatches(const std::vector<std::pair<int, int>>& matches);

    void syncGemstonePositions();

    void setDifficulty(int diff);
    int getDifficulty() const;

    void backToLastGemstoneState(std::string LastState);
    void checkLastGemState();  // 新增

    void updateLevelDisplay();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QVector3D getPosition(int row, int col) const;
    void handleGemstoneClicked(Gemstone* gem);
    void handleManualClick(const QPoint& screenPos , int kind); // 手动处理点击
    void appendDebug(const QString& text);
    void refreshDebugStatus();

    void resetInactivityTimer();

    void updateScoreBoard();
    void updateTimeBoard();
    void triggerFinishIfNeeded();
    void finishToNextLevel();

    void pushInLastStateQueue();
    void generateSimpleMatch();
    bool checkConflict(int x,int y,int type);
    void changeIndex(int a,int b,int c,int d,int e,int f);
    void threeColumnOrVeticalBuild();
    void checkLenthT();
    
    void showFloatingMessage(const QString& text, bool isSuccess);
    void removeFloatingMessage(QLabel* label);

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
    
    int difficulty = 8;
    int GemNumber = 0;
    int Level = 1;
    std::string TempGemState[8];
    int lenthT[8];
    std::stack<std::string> lastGemStateStack;
    bool ConstChange = 0;
    int midX , midY;

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

    // Selection State
    Gemstone* firstSelectedGemstone;
    Gemstone* secondSelectedGemstone;
    
    SelectedCircle* selectionRing1;
    SelectedCircle* selectionRing2;

    QTimer* inactivityTimer;       // 无操作计时器
    int inactivityTimeout = 5000;  // 超时时间(毫秒)，这里设为5秒
    std::vector<SelectedCircle*> highlightRings;  // 用于标记可消除宝石的高亮环
    
    int selectedNum;

    // Swipe/drag state
    bool isDragging;
    QPoint dragStartPos;
    Gemstone* dragStartGemstone;
    int dragStartRow;
    int dragStartCol;
    const int SWIPE_THRESHOLD = 30; // 滑动阈值（像素）

    void setup3DScene();

    // Debug UI
    QTextEdit* debugText;
    QLabel* focusInfoLabel;
    QTimer* debugTimer;

    QWidget* rightPanel = nullptr;
    QLabel* scoreBoardLabel = nullptr;
    QLabel* timeBoardLabel = nullptr;
    QPushButton* backToMenuButton = nullptr;
    QPushButton* resetButton = nullptr;  // 新增
    ScoreProgressBar* scoreProgressBar = nullptr;
    GradientLevelLabel* levelLabel = nullptr;
};

#endif // SINGLE_MODE_GAME_WIDGET_H
