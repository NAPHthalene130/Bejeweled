#ifndef RANK_LIST_WIDGET_H
#define RANK_LIST_WIDGET_H
#include <QWidget>
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
    std::string id;
    int score;
    
    RankRecord() : score(0) {}
    RankRecord(std::string id, int s) : id(id), score(s) {}
};

class RankListWidget : public QWidget {
    Q_OBJECT
public:
    explicit RankListWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    
    // 添加记录的接口
    void setNormalModeRecords(const std::vector<std::pair<std::string, int>>& records);
    void setRotateModeRecords(const std::vector<std::pair<std::string, int>>& records);
    void setMultiplayerRecords(const std::vector<std::pair<std::string, int>>& records);
    
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
    void updateBackgroundAnimation();  // 背景动画更新

private:
    void setupUI();
    void setupTab(QTableWidget* table, const QStringList& headers);
    void updateTable(QTableWidget* table, const std::vector<RankRecord>& records);
    void sortAndKeepTop10(std::vector<RankRecord>& records, bool ascending = false);
    void applyGoldenGlowEffect(QTableWidgetItem* item, int rank);
    QColor getAnimatedGoldColor(int rank, float phase);
    
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
    
    // 背景动画
    float bgAnimPhase = 0.0f;  // 背景动画相位
    QTimer* bgAnimTimer = nullptr;  // 背景动画定时器
    
    // 浮动粒子
    struct Particle {
        float x, y;
        float speedX, speedY;
        float size;
        float alpha;
        float phase;
    };
    std::vector<Particle> particles;
    
    // 海鸥
    struct Seagull {
        float x, y;
        float speed;
        float wingPhase;  // 翅膀扇动相位
        float size;
    };
    std::vector<Seagull> seagulls;
    
    // 鎏金动画
    QTimer* goldenAnimTimer = nullptr;
    float goldenAnimPhase = 0.0f;
    std::vector<QTableWidgetItem*> goldenItems;  // 存储需要动画的item
};

#endif // RANK_LIST_WIDGET_H

