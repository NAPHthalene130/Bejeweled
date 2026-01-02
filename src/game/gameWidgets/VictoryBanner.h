// ============================================================
// VictoryBanner.h
// 放到 gameWidgets 目录下
// ============================================================

#ifndef VICTORY_BANNER_H
#define VICTORY_BANNER_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <QVector>
#include <QPointF>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class VictoryBanner : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal bannerY READ bannerY WRITE setBannerY)
    Q_PROPERTY(qreal bannerOpacity READ bannerOpacity WRITE setBannerOpacity)
    Q_PROPERTY(qreal bannerScale READ bannerScale WRITE setBannerScale)
    Q_PROPERTY(qreal infoOpacity READ infoOpacity WRITE setInfoOpacity)

public:
    explicit VictoryBanner(QWidget* parent = nullptr);
    ~VictoryBanner();

    void show(int level, int score, const QString& time);
    void setVictoryImage(const QString& imagePath);

    // 属性访问器
    qreal bannerY() const { return m_bannerY; }
    void setBannerY(qreal y) { m_bannerY = y; update(); }
    
    qreal bannerOpacity() const { return m_bannerOpacity; }
    void setBannerOpacity(qreal o) { m_bannerOpacity = o; update(); }
    
    qreal bannerScale() const { return m_bannerScale; }
    void setBannerScale(qreal s) { m_bannerScale = s; update(); }
    
    qreal infoOpacity() const { return m_infoOpacity; }
    void setInfoOpacity(qreal o) { m_infoOpacity = o; update(); }

signals:
    void finished();      // 弹幕结束，进入下一关
    void skipRequested(); // 用户点击跳过

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    // 粒子结构
    struct Particle {
        QPointF pos;       // 当前位置
        QPointF velocity;  // 速度
        QPointF target;    // 目标位置（屏幕中心附近）
        qreal size;        // 大小
        qreal opacity;     // 透明度
        qreal rotation;    // 旋转角度
        qreal rotSpeed;    // 旋转速度
        int type;          // 0=星星, 1=圆形, 2=菱形
        QColor color;      // 颜色
        qreal life;        // 生命值 0-1
        qreal maxLife;     // 最大生命
    };

    void initParticles();
    void updateParticles();
    void startAnimations();
    void drawParticle(QPainter& painter, const Particle& p);
    void drawStar(QPainter& painter, const QPointF& center, qreal size, int points = 5);
    void drawDiamond(QPainter& painter, const QPointF& center, qreal size);
    void drawSparkle(QPainter& painter, const QPointF& center, qreal size);

    QPixmap m_victoryImage;
    QTimer* m_particleTimer;
    QTimer* m_autoCloseTimer;
    QVector<Particle> m_particles;
    
    int m_level = 1;
    int m_score = 0;
    QString m_time;
    
    // 动画属性
    qreal m_bannerY = -200;
    qreal m_bannerOpacity = 0;
    qreal m_bannerScale = 0.5;
    qreal m_infoOpacity = 0;
    
    // 背景闪光
    qreal m_flashOpacity = 0;
    int m_frameCount = 0;
    
    // 颜色主题
    QVector<QColor> m_colors;
};

#endif // VICTORY_BANNER_H
