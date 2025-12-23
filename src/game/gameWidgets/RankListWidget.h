#ifndef RANK_LIST_WIDGET_H
#define RANK_LIST_WIDGET_H

#include <QWidget>
#include <QDateTime>
#include <QPixmap>
#include <vector>
#include <QString>

class GameWindow;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QTabWidget;
class QTableWidget;
class QTableWidgetItem;

// 单条排行记录
struct RankRecord {
    int score;              // 分数
    int duration;           // 游戏时长（秒）
    QDateTime playedAt;     // 游戏时间
    QString opponentName;   // 对手名称（仅多人模式）
    bool isWin;             // 是否获胜（仅多人模式）
    
    RankRecord() : score(0), duration(0), isWin(false) {}
    RankRecord(int s, int d, const QDateTime& t, const QString& opp = "", bool win = false)
        : score(s), duration(d), playedAt(t), opponentName(opp), isWin(win) {}
};

class RankListWidget : public QWidget {
    Q_OBJECT
public:
    explicit RankListWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    
    // 添加记录的接口
    void addNormalModeRecord(int score, int duration);
    void addRotateModeRecord(int score, int duration);
    void addMultiplayerRecord(int score, int duration, const QString& opponent, bool isWin);
    
    // 刷新显示
    void refreshDisplay();

signals:
    void backToMenu();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onBackClicked();
    void updateGoldenAnimation();  // 鎏金动画更新

private:
    void setupUI();
    void setupTab(QTableWidget* table, bool isMultiplayer = false);
    void updateTable(QTableWidget* table, const std::vector<RankRecord>& records, bool isMultiplayer = false);
    void sortAndKeepTop10(std::vector<RankRecord>& records);
    void applyGoldenGlowEffect(QTableWidgetItem* item, int rank);
    QColor getAnimatedGoldColor(int rank, float phase);
    QString formatDuration(int seconds) const;
    
    GameWindow* gameWindow = nullptr;
    
    QVBoxLayout* mainLayout = nullptr;
    QPushButton* backButton = nullptr;
    QLabel* titleLabel = nullptr;
    QTabWidget* tabWidget = nullptr;
    
    QTableWidget* normalModeTable = nullptr;
    QTableWidget* rotateModeTable = nullptr;
    QTableWidget* multiplayerTable = nullptr;
    
    // 数据存储
    std::vector<RankRecord> normalModeRecords;
    std::vector<RankRecord> rotateModeRecords;
    std::vector<RankRecord> multiplayerRecords;
    
    // 背景图片
    QPixmap bgImage;
    
    // 鎏金动画
    QTimer* goldenAnimTimer = nullptr;
    float goldenAnimPhase = 0.0f;
    std::vector<QTableWidgetItem*> goldenItems;  // 存储需要动画的item
};

#endif // RANK_LIST_WIDGET_H