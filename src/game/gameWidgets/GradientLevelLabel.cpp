#include "GradientLevelLabel.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <cmath>

GradientLevelLabel::GradientLevelLabel(QWidget* parent)
    : QWidget(parent)
    , m_level(1)
    , m_startColor(255, 150, 200)    // 粉色
    , m_endColor(180, 100, 255)      // 紫色
    , m_highlightPosition(0.0)
    , m_animationEnabled(true)
{
    // 设置透明背景
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(350, 120);  // 【修改】进一步增大最小尺寸
    
    // 初始化高光动画定时器
    m_highlightTimer = new QTimer(this);
    m_highlightTimer->setInterval(20);  // 约50fps
    connect(m_highlightTimer, &QTimer::timeout, this, &GradientLevelLabel::updateHighlightPosition);
    
    if (m_animationEnabled) {
        m_highlightTimer->start();
    }
}

GradientLevelLabel::~GradientLevelLabel() {
    if (m_highlightTimer) {
        m_highlightTimer->stop();
    }
}

void GradientLevelLabel::setLevel(int level) {
    if (m_level != level) {
        m_level = level;
        update();
    }
}

int GradientLevelLabel::getLevel() const {
    return m_level;
}

void GradientLevelLabel::setGradientColors(const QColor& startColor, const QColor& endColor) {
    m_startColor = startColor;
    m_endColor = endColor;
    update();
}

void GradientLevelLabel::setAnimationEnabled(bool enabled) {
    m_animationEnabled = enabled;
    if (enabled) {
        m_highlightTimer->start();
    } else {
        m_highlightTimer->stop();
    }
}

void GradientLevelLabel::updateHighlightPosition() {
    // 高光从左到右移动，然后循环
    m_highlightPosition += 0.018;  // 高光移动速度
    if (m_highlightPosition > 1.5) {
        m_highlightPosition = -0.5;
    }
    update();
}

QSize GradientLevelLabel::sizeHint() const {
    return QSize(380, 130);  // 【修改】进一步增大建议尺寸
}

QSize GradientLevelLabel::minimumSizeHint() const {
    return QSize(340, 110);  // 【修改】进一步增大最小尺寸
}

void GradientLevelLabel::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // 构建显示文本
    QString levelText = "Level";
    QString numberText = QString::number(m_level);
    
    // 【修改】设置字体 - "Level" 使用更大字体（原来是32，现在是42）
    QFont levelFont;
    levelFont.setFamily("Microsoft YaHei");
    levelFont.setPointSize(42);  // 进一步放大字体
    levelFont.setBold(true);
    levelFont.setLetterSpacing(QFont::PercentageSpacing, 105);
    
    // 【修改】数字使用更大的字体（原来是40，现在是52）
    QFont numberFont;
    numberFont.setFamily("Microsoft YaHei");
    numberFont.setPointSize(52);  // 进一步放大数字字体
    numberFont.setBold(true);
    
    QFontMetrics levelMetrics(levelFont);
    QFontMetrics numberMetrics(numberFont);
    
    int levelWidth = levelMetrics.horizontalAdvance(levelText);
    int numberWidth = numberMetrics.horizontalAdvance(numberText);
    int spacing = 20;  // 【修改】增大Level和数字之间的间距（原来是16）
    int totalWidth = levelWidth + spacing + numberWidth;
    
    // 计算起始位置（居中）
    int startX = (width() - totalWidth) / 2;
    int levelY = height() / 2 + levelMetrics.ascent() / 2 - 5;
    int numberY = height() / 2 + numberMetrics.ascent() / 2 - 5;
    
    // ============ 绘制 "Level" 文字 ============
    
    // 1. 绘制阴影层（多层阴影增加立体感）
    painter.setFont(levelFont);
    
    // 【修改】深色阴影 - 增大偏移
    painter.setPen(QColor(80, 40, 100, 120));
    painter.drawText(startX + 5, levelY + 5, levelText);
    
    // 【修改】中等阴影 - 增大偏移
    painter.setPen(QColor(100, 60, 140, 100));
    painter.drawText(startX + 3, levelY + 3, levelText);
    
    // 2. 创建渐变画笔绘制主文字
    QLinearGradient gradient(startX, levelY - levelMetrics.ascent(), 
                             startX, levelY + levelMetrics.descent());
    gradient.setColorAt(0.0, m_startColor);      // 顶部粉色
    gradient.setColorAt(0.5, QColor(230, 130, 220));  // 中间过渡
    gradient.setColorAt(1.0, m_endColor);        // 底部紫色
    
    // 绘制文字轮廓（增加立体感）
    QPainterPath levelPath;
    levelPath.addText(startX, levelY, levelFont, levelText);
    
    // 【修改】绘制外发光效果 - 增大光晕
    QPen glowPen(QColor(255, 200, 255, 60), 6);
    painter.strokePath(levelPath, glowPen);
    
    // 绘制渐变填充的主文字
    painter.fillPath(levelPath, gradient);
    
    // 3. 绘制高光效果
    if (m_animationEnabled) {
        // 【修改】创建更宽的高光渐变
        qreal highlightX = startX + (levelWidth + spacing + numberWidth) * m_highlightPosition;
        QLinearGradient highlightGradient(highlightX - 50, 0, highlightX + 50, 0);
        highlightGradient.setColorAt(0.0, QColor(255, 255, 255, 0));
        highlightGradient.setColorAt(0.5, QColor(255, 255, 255, 200));
        highlightGradient.setColorAt(1.0, QColor(255, 255, 255, 0));
        
        // 裁剪到文字区域
        painter.save();
        painter.setClipPath(levelPath);
        painter.fillRect(rect(), highlightGradient);
        painter.restore();
    }
    
    // ============ 绘制数字 ============
    
    int numberX = startX + levelWidth + spacing;
    
    painter.setFont(numberFont);
    
    // 【修改】数字阴影 - 增大偏移
    painter.setPen(QColor(80, 40, 100, 120));
    painter.drawText(numberX + 5, numberY + 5, numberText);
    
    painter.setPen(QColor(100, 60, 140, 100));
    painter.drawText(numberX + 3, numberY + 3, numberText);
    
    // 数字渐变
    QLinearGradient numberGradient(numberX, numberY - numberMetrics.ascent(),
                                   numberX, numberY + numberMetrics.descent());
    numberGradient.setColorAt(0.0, m_startColor);
    numberGradient.setColorAt(0.5, QColor(230, 130, 220));
    numberGradient.setColorAt(1.0, m_endColor);
    
    // 数字路径
    QPainterPath numberPath;
    numberPath.addText(numberX, numberY, numberFont, numberText);
    
    // 数字外发光
    painter.strokePath(numberPath, glowPen);
    
    // 渐变填充数字
    painter.fillPath(numberPath, numberGradient);
    
    // 数字高光
    if (m_animationEnabled) {
        qreal highlightX = startX + (levelWidth + spacing + numberWidth) * m_highlightPosition;
        QLinearGradient highlightGradient(highlightX - 50, 0, highlightX + 50, 0);
        highlightGradient.setColorAt(0.0, QColor(255, 255, 255, 0));
        highlightGradient.setColorAt(0.5, QColor(255, 255, 255, 200));
        highlightGradient.setColorAt(1.0, QColor(255, 255, 255, 0));
        
        painter.save();
        painter.setClipPath(numberPath);
        painter.fillRect(rect(), highlightGradient);
        painter.restore();
    }
    
    // ============ 可选：绘制装饰性下划线 ============
    
    // 【修改】渐变下划线 - 增大偏移和粗细
    int underlineY = levelY + 22;
    QLinearGradient underlineGradient(startX, 0, startX + totalWidth, 0);
    underlineGradient.setColorAt(0.0, QColor(255, 150, 200, 200));
    underlineGradient.setColorAt(0.5, QColor(220, 130, 230, 200));
    underlineGradient.setColorAt(1.0, QColor(180, 100, 255, 200));
    
    QPen underlinePen(QBrush(underlineGradient), 4);  // 增粗下划线
    underlinePen.setCapStyle(Qt::RoundCap);
    painter.setPen(underlinePen);
    painter.drawLine(startX, underlineY, startX + totalWidth, underlineY);
}
