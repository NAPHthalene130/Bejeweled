#ifndef GRADIENT_LEVEL_LABEL_H
#define GRADIENT_LEVEL_LABEL_H

#include <QWidget>
#include <QTimer>
#include <QString>

/**
 * GradientLevelLabel - 带渐变色、阴影和高光动画的Level标签
 * 
 * 特性：
 * - 粉色到紫色的渐变文字
 * - 3D阴影效果
 * - 平滑移动的高光动画
 */
class GradientLevelLabel : public QWidget {
    Q_OBJECT

public:
    explicit GradientLevelLabel(QWidget* parent = nullptr);
    ~GradientLevelLabel();

    void setLevel(int level);
    int getLevel() const;

    // 自定义颜色（可选）
    void setGradientColors(const QColor& startColor, const QColor& endColor);
    
    // 启用/禁用高光动画
    void setAnimationEnabled(bool enabled);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void updateHighlightPosition();

private:
    int m_level;
    QColor m_startColor;   // 渐变起始色（粉色）
    QColor m_endColor;     // 渐变结束色（紫色）
    
    // 高光动画相关
    QTimer* m_highlightTimer;
    qreal m_highlightPosition;  // 高光位置 0.0 ~ 1.0
    bool m_animationEnabled;
    
    // 绘制辅助
    void drawTextWithEffects(QPainter& painter, const QString& text, const QRect& rect);
    void drawHighlight(QPainter& painter, const QString& text, const QRect& rect);
};

#endif // GRADIENT_LEVEL_LABEL_H
