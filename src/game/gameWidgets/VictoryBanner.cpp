// ============================================================
// VictoryBanner.cpp
// 放到 gameWidgets 目录下
// ============================================================

#include "VictoryBanner.h"
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QtMath>
#include <cmath>

VictoryBanner::VictoryBanner(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setMouseTracking(true);
    
    // 初始化颜色主题（紫色、蓝色、金色系）
    m_colors = {
        QColor(180, 100, 255),   // 紫色
        QColor(130, 80, 255),    // 深紫
        QColor(100, 150, 255),   // 蓝色
        QColor(255, 215, 0),     // 金色
        QColor(255, 180, 50),    // 橙金
        QColor(255, 255, 150),   // 亮黄
        QColor(200, 150, 255),   // 淡紫
        QColor(150, 200, 255),   // 淡蓝
    };
    
    // 粒子更新定时器 (60fps)
    m_particleTimer = new QTimer(this);
    m_particleTimer->setInterval(16);
    connect(m_particleTimer, &QTimer::timeout, this, [this]() {
        updateParticles();
        update();
    });
    
    // 自动关闭定时器
    m_autoCloseTimer = new QTimer(this);
    m_autoCloseTimer->setSingleShot(true);
    connect(m_autoCloseTimer, &QTimer::timeout, this, [this]() {
        emit finished();
        close();
    });
}

VictoryBanner::~VictoryBanner() {
    m_particleTimer->stop();
    m_autoCloseTimer->stop();
}

void VictoryBanner::setVictoryImage(const QString& imagePath) {
    m_victoryImage.load(imagePath);
}

void VictoryBanner::show(int level, int score, const QString& time) {
    m_level = level;
    m_score = score;
    m_time = time;
    
    // 获取父窗口的全局位置和大小
    if (parentWidget()) {
        QWidget* topLevel = parentWidget()->window();  // 获取顶层窗口
        setGeometry(topLevel->geometry());  // 覆盖整个窗口
    }
    
    QWidget::show();
    raise();
    activateWindow();  // 确保获得焦点
    
    initParticles();
    startAnimations();
    m_particleTimer->start();
    
    m_autoCloseTimer->start(5000);
}


void VictoryBanner::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (parentWidget()) {
        setGeometry(parentWidget()->rect());
    }
}

void VictoryBanner::initParticles() {
    m_particles.clear();
    
    int w = width();
    int h = height();
    QPointF center(w / 2.0, h / 2.0);
    
    // 创建从四周飞入的粒子
    for (int i = 0; i < 80; ++i) {
        Particle p;
        
        // 随机选择边缘位置
        int edge = QRandomGenerator::global()->bounded(4);
        switch (edge) {
            case 0: // 上边
                p.pos = QPointF(QRandomGenerator::global()->bounded(w), -50);
                break;
            case 1: // 下边
                p.pos = QPointF(QRandomGenerator::global()->bounded(w), h + 50);
                break;
            case 2: // 左边
                p.pos = QPointF(-50, QRandomGenerator::global()->bounded(h));
                break;
            case 3: // 右边
                p.pos = QPointF(w + 50, QRandomGenerator::global()->bounded(h));
                break;
        }
        
        // 目标位置在屏幕中心区域（有随机偏移）
        p.target = QPointF(
            center.x() + QRandomGenerator::global()->bounded(-300, 300),
            center.y() + QRandomGenerator::global()->bounded(-200, 200)
        );
        
        // 计算初始速度（朝向目标）
        QPointF dir = p.target - p.pos;
        qreal dist = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
        if (dist > 0) {
            dir /= dist;
        }
        qreal speed = 8 + QRandomGenerator::global()->bounded(8);
        p.velocity = dir * speed;
        
        p.size = 8 + QRandomGenerator::global()->bounded(20);
        p.opacity = 0.6 + QRandomGenerator::global()->bounded(40) / 100.0;
        p.rotation = QRandomGenerator::global()->bounded(360);
        p.rotSpeed = -5 + QRandomGenerator::global()->bounded(10);
        p.type = QRandomGenerator::global()->bounded(3);
        p.color = m_colors[QRandomGenerator::global()->bounded(m_colors.size())];
        p.life = 1.0;
        p.maxLife = 2.0 + QRandomGenerator::global()->bounded(20) / 10.0;
        
        m_particles.append(p);
    }
    
    // 添加一些持续生成的环绕粒子
    for (int i = 0; i < 30; ++i) {
        Particle p;
        qreal angle = QRandomGenerator::global()->bounded(360) * M_PI / 180.0;
        qreal radius = 150 + QRandomGenerator::global()->bounded(200);
        
        p.pos = QPointF(
            center.x() + std::cos(angle) * radius,
            center.y() + std::sin(angle) * radius
        );
        p.target = center;
        p.velocity = QPointF(
            std::cos(angle + M_PI/2) * 2,
            std::sin(angle + M_PI/2) * 2
        );
        p.size = 5 + QRandomGenerator::global()->bounded(10);
        p.opacity = 0.4 + QRandomGenerator::global()->bounded(40) / 100.0;
        p.rotation = 0;
        p.rotSpeed = 3;
        p.type = QRandomGenerator::global()->bounded(3);
        p.color = m_colors[QRandomGenerator::global()->bounded(m_colors.size())];
        p.life = 0.5 + QRandomGenerator::global()->bounded(50) / 100.0;
        p.maxLife = 3.0;
        
        m_particles.append(p);
    }
}

void VictoryBanner::updateParticles() {
    m_frameCount++;
    QPointF center(width() / 2.0, height() / 2.0);
    
    for (int i = m_particles.size() - 1; i >= 0; --i) {
        Particle& p = m_particles[i];
        
        // 更新位置
        p.pos += p.velocity;
        
        // 接近目标时减速
        QPointF toTarget = p.target - p.pos;
        qreal dist = std::sqrt(toTarget.x() * toTarget.x() + toTarget.y() * toTarget.y());
        if (dist < 100) {
            p.velocity *= 0.95;
            // 添加环绕运动
            p.velocity += QPointF(-toTarget.y(), toTarget.x()) * 0.02;
        }
        
        // 更新旋转
        p.rotation += p.rotSpeed;
        
        // 更新生命值
        p.life -= 0.008;
        
        // 移除死亡粒子并生成新粒子
        if (p.life <= 0) {
            // 在边缘重新生成
            int edge = QRandomGenerator::global()->bounded(4);
            switch (edge) {
                case 0:
                    p.pos = QPointF(QRandomGenerator::global()->bounded(width()), -30);
                    break;
                case 1:
                    p.pos = QPointF(QRandomGenerator::global()->bounded(width()), height() + 30);
                    break;
                case 2:
                    p.pos = QPointF(-30, QRandomGenerator::global()->bounded(height()));
                    break;
                case 3:
                    p.pos = QPointF(width() + 30, QRandomGenerator::global()->bounded(height()));
                    break;
            }
            
            p.target = QPointF(
                center.x() + QRandomGenerator::global()->bounded(-250, 250),
                center.y() + QRandomGenerator::global()->bounded(-150, 150)
            );
            
            QPointF dir = p.target - p.pos;
            qreal d = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
            if (d > 0) dir /= d;
            p.velocity = dir * (6 + QRandomGenerator::global()->bounded(6));
            p.life = 1.0;
            p.size = 6 + QRandomGenerator::global()->bounded(18);
            p.color = m_colors[QRandomGenerator::global()->bounded(m_colors.size())];
        }
    }
    
    // 更新背景闪光
    m_flashOpacity = 0.1 + 0.05 * std::sin(m_frameCount * 0.1);
}

void VictoryBanner::startAnimations() {
    // 初始化属性
    m_bannerY = -300;
    m_bannerOpacity = 0;
    m_bannerScale = 0.3;
    m_infoOpacity = 0;
    
    // 图片从上方弹入动画
    QPropertyAnimation* yAnim = new QPropertyAnimation(this, "bannerY");
    yAnim->setDuration(800);
    yAnim->setStartValue(-300.0);
    yAnim->setEndValue(height() / 2.0 - 100);
    yAnim->setEasingCurve(QEasingCurve::OutBack);
    
    // 图片淡入
    QPropertyAnimation* opacityAnim = new QPropertyAnimation(this, "bannerOpacity");
    opacityAnim->setDuration(600);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);
    opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // 图片缩放
    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "bannerScale");
    scaleAnim->setDuration(800);
    scaleAnim->setStartValue(0.3);
    scaleAnim->setEndValue(1.0);
    scaleAnim->setEasingCurve(QEasingCurve::OutBack);
    
    // 信息文字淡入（延迟）
    QPropertyAnimation* infoAnim = new QPropertyAnimation(this, "infoOpacity");
    infoAnim->setDuration(500);
    infoAnim->setStartValue(0.0);
    infoAnim->setEndValue(1.0);
    infoAnim->setEasingCurve(QEasingCurve::OutCubic);
    
    // 组合动画
    QParallelAnimationGroup* mainGroup = new QParallelAnimationGroup(this);
    mainGroup->addAnimation(yAnim);
    mainGroup->addAnimation(opacityAnim);
    mainGroup->addAnimation(scaleAnim);
    
    QSequentialAnimationGroup* seqGroup = new QSequentialAnimationGroup(this);
    seqGroup->addAnimation(mainGroup);
    seqGroup->addPause(200);
    seqGroup->addAnimation(infoAnim);
    
    seqGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void VictoryBanner::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    int w = width();
    int h = height();
    QPointF center(w / 2.0, h / 2.0);
    
    // 1. 绘制半透明背景
    QColor bgColor(10, 5, 20, 180);
    painter.fillRect(rect(), bgColor);
    
    // 2. 绘制中心光晕
    QRadialGradient glow(center, 400);
    glow.setColorAt(0, QColor(150, 100, 255, int(60 * m_flashOpacity * 3)));
    glow.setColorAt(0.5, QColor(100, 50, 200, int(30 * m_flashOpacity * 3)));
    glow.setColorAt(1, Qt::transparent);
    painter.fillRect(rect(), glow);
    
    // 3. 绘制粒子（在图片后面）
    for (const Particle& p : m_particles) {
        if (p.life > 0) {
            drawParticle(painter, p);
        }
    }
    
    // 4. 绘制胜利图片
    if (!m_victoryImage.isNull() && m_bannerOpacity > 0) {
        painter.save();
        painter.setOpacity(m_bannerOpacity);
        
        // 计算图片位置和大小
        qreal imgW = m_victoryImage.width() * m_bannerScale * 0.8;
        qreal imgH = m_victoryImage.height() * m_bannerScale * 0.8;
        qreal imgX = center.x() - imgW / 2;
        qreal imgY = m_bannerY - imgH / 2;
        
        // 绘制图片发光效果
        painter.setOpacity(m_bannerOpacity * 0.3);
        for (int i = 3; i >= 1; --i) {
            QRectF glowRect(imgX - i * 8, imgY - i * 8, imgW + i * 16, imgH + i * 16);
            painter.drawPixmap(glowRect.toRect(), m_victoryImage.scaled(
                glowRect.width(), glowRect.height(), 
                Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        
        // 绘制主图片
        painter.setOpacity(m_bannerOpacity);
        painter.drawPixmap(QRectF(imgX, imgY, imgW, imgH).toRect(), 
            m_victoryImage.scaled(imgW, imgH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        
        painter.restore();
    }
    
    // 5. 绘制关卡信息
    if (m_infoOpacity > 0) {
        painter.save();
        painter.setOpacity(m_infoOpacity);
        
        qreal infoY = m_bannerY + 80;
        
        // 信息背景卡片
        QRectF infoRect(center.x() - 180, infoY, 360, 120);
        
        // 绘制毛玻璃效果背景
        QPainterPath cardPath;
        cardPath.addRoundedRect(infoRect, 16, 16);
        
        QLinearGradient cardBg(infoRect.topLeft(), infoRect.bottomRight());
        cardBg.setColorAt(0, QColor(80, 50, 120, 180));
        cardBg.setColorAt(1, QColor(40, 30, 80, 180));
        painter.fillPath(cardPath, cardBg);
        
        // 金色边框
        QPen borderPen(QColor(255, 200, 100, 150));
        borderPen.setWidth(2);
        painter.setPen(borderPen);
        painter.drawPath(cardPath);
        
        // 关卡文字
        QFont levelFont("Microsoft YaHei", 22, QFont::Bold);
        painter.setFont(levelFont);
        painter.setPen(QColor(255, 215, 0));
        painter.drawText(QRectF(infoRect.x(), infoY + 10, infoRect.width(), 40),
                        Qt::AlignCenter, QString("第 %1 关 完成！").arg(m_level));
        
        // 得分和时间
        QFont detailFont("Microsoft YaHei", 14);
        painter.setFont(detailFont);
        painter.setPen(QColor(230, 230, 255));
        painter.drawText(QRectF(infoRect.x(), infoY + 55, infoRect.width() / 2, 30),
                        Qt::AlignCenter, QString("🏆 %1 分").arg(m_score));
        painter.drawText(QRectF(infoRect.x() + infoRect.width() / 2, infoY + 55, infoRect.width() / 2, 30),
                        Qt::AlignCenter, QString("⏱️ %1").arg(m_time));
        
        // 提示文字
        QFont tipFont("Microsoft YaHei", 11);
        painter.setFont(tipFont);
        painter.setPen(QColor(180, 180, 200, 180));
        painter.drawText(QRectF(infoRect.x(), infoY + 90, infoRect.width(), 25),
                        Qt::AlignCenter, "点击任意处继续...");
        
        painter.restore();
    }
    
    // 6. 在最前面绘制一些额外的闪光粒子
    for (int i = 0; i < 5; ++i) {
        qreal angle = (m_frameCount * 2 + i * 72) * M_PI / 180.0;
        qreal radius = 200 + 20 * std::sin(m_frameCount * 0.05 + i);
        QPointF sparkPos(
            center.x() + std::cos(angle) * radius,
            m_bannerY + std::sin(angle) * radius * 0.3
        );
        
        painter.save();
        painter.setOpacity(0.6 + 0.3 * std::sin(m_frameCount * 0.1 + i));
        drawSparkle(painter, sparkPos, 15 + 5 * std::sin(m_frameCount * 0.15 + i));
        painter.restore();
    }
}

void VictoryBanner::drawParticle(QPainter& painter, const Particle& p) {
    painter.save();
    painter.translate(p.pos);
    painter.rotate(p.rotation);
    painter.setOpacity(p.opacity * p.life);
    
    QColor c = p.color;
    
    switch (p.type) {
        case 0: // 星星
            drawStar(painter, QPointF(0, 0), p.size);
            break;
        case 1: // 圆形光点
            {
                QRadialGradient grad(0, 0, p.size);
                grad.setColorAt(0, c);
                grad.setColorAt(0.5, QColor(c.red(), c.green(), c.blue(), 150));
                grad.setColorAt(1, Qt::transparent);
                painter.setBrush(grad);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPointF(0, 0), p.size, p.size);
            }
            break;
        case 2: // 菱形
            drawDiamond(painter, QPointF(0, 0), p.size);
            break;
    }
    
    painter.restore();
}

void VictoryBanner::drawStar(QPainter& painter, const QPointF& center, qreal size, int points) {
    QPainterPath path;
    qreal innerRadius = size * 0.4;
    qreal outerRadius = size;
    
    for (int i = 0; i < points * 2; ++i) {
        qreal radius = (i % 2 == 0) ? outerRadius : innerRadius;
        qreal angle = (i * M_PI / points) - M_PI / 2;
        QPointF pt(center.x() + radius * std::cos(angle),
                   center.y() + radius * std::sin(angle));
        if (i == 0) {
            path.moveTo(pt);
        } else {
            path.lineTo(pt);
        }
    }
    path.closeSubpath();
    
    // 渐变填充
    QRadialGradient grad(center, size);
    grad.setColorAt(0, QColor(255, 255, 200));
    grad.setColorAt(0.3, QColor(255, 220, 100));
    grad.setColorAt(1, QColor(255, 180, 50, 100));
    
    painter.setBrush(grad);
    painter.setPen(QPen(QColor(255, 255, 255, 150), 1));
    painter.drawPath(path);
}

void VictoryBanner::drawDiamond(QPainter& painter, const QPointF& center, qreal size) {
    QPainterPath path;
    path.moveTo(center.x(), center.y() - size);
    path.lineTo(center.x() + size * 0.6, center.y());
    path.lineTo(center.x(), center.y() + size);
    path.lineTo(center.x() - size * 0.6, center.y());
    path.closeSubpath();
    
    QLinearGradient grad(center.x(), center.y() - size, center.x(), center.y() + size);
    grad.setColorAt(0, QColor(200, 150, 255));
    grad.setColorAt(0.5, QColor(150, 100, 255));
    grad.setColorAt(1, QColor(100, 50, 200));
    
    painter.setBrush(grad);
    painter.setPen(QPen(QColor(255, 255, 255, 100), 1));
    painter.drawPath(path);
}

void VictoryBanner::drawSparkle(QPainter& painter, const QPointF& center, qreal size) {
    // 十字闪光
    painter.setPen(QPen(QColor(255, 255, 255, 200), 2));
    painter.drawLine(QPointF(center.x() - size, center.y()),
                     QPointF(center.x() + size, center.y()));
    painter.drawLine(QPointF(center.x(), center.y() - size),
                     QPointF(center.x(), center.y() + size));
    
    // 对角线（较短）
    painter.setPen(QPen(QColor(255, 255, 200, 150), 1.5));
    qreal diag = size * 0.6;
    painter.drawLine(QPointF(center.x() - diag, center.y() - diag),
                     QPointF(center.x() + diag, center.y() + diag));
    painter.drawLine(QPointF(center.x() + diag, center.y() - diag),
                     QPointF(center.x() - diag, center.y() + diag));
    
    // 中心光点
    QRadialGradient glow(center, size * 0.3);
    glow.setColorAt(0, QColor(255, 255, 255, 255));
    glow.setColorAt(1, Qt::transparent);
    painter.setBrush(glow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center, size * 0.3, size * 0.3);
}

void VictoryBanner::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    // 点击任意处跳过
    m_autoCloseTimer->stop();
    m_particleTimer->stop();
    emit finished();
    close();
}
