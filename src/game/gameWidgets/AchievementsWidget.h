#include <QPainter>
#include <random>
#include <QWidget>
#include <QPaintEvent>
#include <QTimer>
#include <vector>

// 动态星星粒子
struct StarParticle {
    float x, y;           // 位置 (0-1 归一化)
    float size;           // 大小
    float alpha;          // 透明度
    float twinklePhase;   // 闪烁相位
    float twinkleSpeed;   // 闪烁速度
    float driftX, driftY; // 飘动速度
};

// 背景装饰层：绘制星星和宝石
class AchievementsBackgroundDecoration : public QWidget {
    protected:
        void paintEvent(QPaintEvent*) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void leaveEvent(QEvent* event) override;
    Q_OBJECT
public:
    explicit AchievementsBackgroundDecoration(QWidget* parent = nullptr);
    void setContentMargin(int margin) { contentMargin = margin; }
private slots:
    void updateAnimation();
private:
    void initParticles();
    int contentMargin = 0;
    QPixmap userBg;
    QPixmap bgImage; // 背景图片
    bool starShy = false;
    QRect starRectCache;
    // 动画系统
    QTimer* animTimer = nullptr;
    std::vector<StarParticle> particles;
    float animTime = 0;
};
#ifndef ACHIEVEMENTS_WIDGET_H
#define ACHIEVEMENTS_WIDGET_H
#include <QWidget>

class GameWindow;
class QVBoxLayout;
class QGridLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QPixmap;
class QMediaPlayer;
class QAudioOutput;

namespace Qt3DExtras { class Qt3DWindow; }
namespace Qt3DCore { class QEntity; }

class AchievementsWidget : public QWidget {
    Q_OBJECT
public:
    explicit AchievementsWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    // Force UI to reload achievements from GameWindow
    void updateView();

signals:
    void backToMenu();

private slots:
    void onPrevPage();
    void onNextPage();
    void onBackClicked();

private:
    void refresh();

    GameWindow* gameWindow = nullptr;

    QVBoxLayout* mainLayout = nullptr;
    QWidget* gridContainer = nullptr;
    QGridLayout* gridLayout = nullptr;
    QPixmap bgPixmap;
    QLabel* bgLabel = nullptr;

    // Qt3D background (always enabled)
    Qt3DExtras::Qt3DWindow* qt3dWindow = nullptr;
    QWidget* qt3dContainer = nullptr;
    Qt3DCore::QEntity* qt3dRoot = nullptr;
    QHBoxLayout* navLayout = nullptr;
    QPushButton* prevButton = nullptr;
    QPushButton* nextButton = nullptr;
    QLabel* pageLabel = nullptr;
    QPushButton* backButton = nullptr;

    int currentPage = 0;
    int itemsPerPage = 6; // 3 columns x 2 rows (default pagination)
    
    // 背景音乐
    QMediaPlayer* bgMusic = nullptr;
    QAudioOutput* audioOutput = nullptr;
protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void updateBackground();
};
#endif // ACHIEVEMENTS_WIDGET_H

