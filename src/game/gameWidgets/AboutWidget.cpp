#include "AboutWidget.h"
#include "../GameWindow.h"
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QScroller>
#include <QRandomGenerator>
#include <cmath>

AboutWidget::AboutWidget(QWidget *parent, GameWindow *gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    setupUI();
    setupAnimations();
    
    // Load background image
    backgroundPixmap.load("D:/HuaweiMoveData/Users/祝霖瑞/Desktop/微信图片_20260101142040.png");

    // Background animation timer (only for border hue now)
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        bgHue += 0.2f;
        if (bgHue >= 360.0f) bgHue -= 360.0f;
        
        time += 0.02f; // Update time for sine wave
        updateSakuras();
        update();
    });
    timer->start(16); // ~60 FPS
}

void AboutWidget::setupUI() {
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Content Box (The semi-transparent container)
    contentBox = new QWidget(this);
    contentBox->setFixedSize(900, 700);
    
    // Layout for content box
    QVBoxLayout *boxLayout = new QVBoxLayout(contentBox);
    boxLayout->setContentsMargins(50, 120, 50, 50); // Increased top margin for manual title drawing
    boxLayout->setSpacing(30);

    // Title (Removed QLabel, drawing manually in paintEvent)
    
    // Scrollable Text Display
    textDisplay = new QTextEdit(contentBox);
    textDisplay->setReadOnly(true);
    textDisplay->setFrameShape(QFrame::NoFrame);
    textDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textDisplay->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // Custom Scrollbar Style
    textDisplay->setStyleSheet(
        "QTextEdit { background: transparent; color: #E0E0E0; font-size: 20px; font-family: 'Microsoft YaHei', 'Segoe UI', sans-serif; selection-background-color: #00FFFF; selection-color: #000000; }"
        "QScrollBar:vertical { border: none; background: rgba(0,0,0,0.2); width: 8px; margin: 0px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: rgba(255,255,255,0.4); min-height: 20px; border-radius: 4px; }"
        "QScrollBar::handle:vertical:hover { background: rgba(255,255,255,0.7); }"
        "QScrollBar::add-line:vertical { height: 0px; }"
        "QScrollBar::sub-line:vertical { height: 0px; }"
    );
    
    // Set HTML content with rich styling
    // Using FangSong (仿宋) for artistic effect on main content as requested
    QString htmlContent = R"(
        <div style='text-align: center; line-height: 1.6; font-family: "FangSong", "STFangSong", "SimSun", serif;'>
            <!-- Version Section: Cyan Theme -->
            <h2 style='color: #00FFFF; margin-bottom: 5px; font-size: 36px; text-shadow: 0px 0px 5px #00FFFF;'>版本 1.0.0</h2>
            <br>
            <p style='font-size: 28px; color: #E0FFFF;'>经典三消益智游戏的现代高性能重制版。</p>
            <br>
            <hr style='border: 1px solid rgba(255,255,255,0.1); margin: 20px 0;'>
            <br>
            
            <!-- Team Section: Magenta Theme -->
            <h3 style='color: #FF00FF; letter-spacing: 2px; font-size: 32px; text-shadow: 0px 0px 5px #FF00FF;'>开发团队</h3>
            <p style='font-size: 36px; font-weight: bold; color: #FFB6C1;'>第二组</p>
            <br>
            
            <!-- Tech Stack Section: Yellow Theme -->
            <h3 style='color: #FFFF00; letter-spacing: 2px; font-size: 32px; text-shadow: 0px 0px 5px #FFFF00;'>技术栈</h3>
            <div style='font-size: 24px; color: #FFFACD;'>
                <p><b>核心:</b> C++17</p>
                <p><b>框架:</b> Qt 6.10</p>
                <p><b>渲染:</b> Qt 3D & OpenGL</p>
                <p><b>音频:</b> Qt Multimedia</p>
            </div>
            <br>
            <hr style='border: 1px solid rgba(255,255,255,0.1); margin: 20px 0;'>
            <br>
            
            <!-- Special Thanks: Green Theme -->
            <h3 style='color: #00FF00; letter-spacing: 2px; font-size: 32px; text-shadow: 0px 0px 5px #00FF00;'>特别鸣谢</h3>
            <p style='font-size: 24px; color: #98FB98;'>致所有开源贡献者和游戏社区。</p>
            <br>
            
            <!-- Quote: Standard Font (Excluded from artistic effect) -->
            <p style='font-family: "Microsoft YaHei", sans-serif; font-style: italic; color: #888888; font-size: 18px;'>"简约是极致的复杂。"</p>
            <br><br>
            
            <!-- Copyright: Standard Font (Excluded from artistic effect) -->
            <p style='font-family: "Microsoft YaHei", sans-serif; font-size: 14px; color: #666666;'>© 2026 宝石迷阵项目组。保留所有权利。</p>
            <br><br>
        </div>
    )";
    textDisplay->setHtml(htmlContent);
    
    // Enable touch scrolling (kinetic scrolling)
    QScroller::grabGesture(textDisplay->viewport(), QScroller::LeftMouseButtonGesture);

    // Back Button
    backButton = new QPushButton("返回主菜单", contentBox);
    backButton->setFixedSize(260, 60);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(
        "QPushButton { "
        "   background-color: rgba(255, 255, 255, 0.05); "
        "   color: white; "
        "   border: 1px solid rgba(255, 255, 255, 0.3); "
        "   border-radius: 30px; "
        "   font-size: 18px; "
        "   font-weight: bold; "
        "   font-family: 'Microsoft YaHei'; "
        "   letter-spacing: 2px; "
        "}"
        "QPushButton:hover { "
        "   background-color: rgba(255, 255, 255, 0.2); "
        "   border-color: #00FFFF; "
        "   color: #00FFFF; "
        "}"
        "QPushButton:pressed { "
        "   background-color: rgba(0, 255, 255, 0.2); "
        "   border-color: #00FFFF; "
        "}"
    );
    connect(backButton, &QPushButton::clicked, this, &AboutWidget::backToMenu);

    // Add widgets to box layout
    // boxLayout->addWidget(titleLabel); // Removed
    boxLayout->addWidget(textDisplay);
    boxLayout->addWidget(backButton, 0, Qt::AlignHCenter);

    mainLayout->addWidget(contentBox);
}

void AboutWidget::setupAnimations() {
    opacityEffect = new QGraphicsOpacityEffect(contentBox);
    contentBox->setGraphicsEffect(opacityEffect);

    entryAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    entryAnimation->setDuration(800);
    entryAnimation->setStartValue(0.0);
    entryAnimation->setEndValue(1.0);
    entryAnimation->setEasingCurve(QEasingCurve::OutBack);
}

void AboutWidget::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    entryAnimation->start();
}

void AboutWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // 1. Draw Background Image with Sway Effect
    if (!backgroundPixmap.isNull()) {
        p.save();
        
        // Calculate sway parameters
        float swayAngle = std::sin(time) * 2.0f; // +/- 2 degrees
        float scaleFactor = 1.05f + std::sin(time * 1.5f) * 0.01f; // Slight breathing scale
        
        // Transform origin to bottom center
        QPointF center(width() / 2.0, height());
        p.translate(center);
        p.rotate(swayAngle);
        p.scale(scaleFactor, scaleFactor);
        p.translate(-center);
        
        // Draw slightly larger than rect to cover edges during rotation
        QRect targetRect = rect().adjusted(-50, -50, 50, 50);
        p.drawPixmap(targetRect, backgroundPixmap.scaled(targetRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        
        p.restore();
    } else {
        p.fillRect(rect(), Qt::black);
    }

    // 1.5 Draw Sakura Particles
    p.setPen(Qt::NoPen);
    for (const auto &s : sakuras) {
        p.save();
        p.translate(s.x, s.y);
        p.rotate(s.rotation);
        
        // Draw Petal (Simple Ellipse for now, or a custom path)
        QColor petalColor(255, 192, 203, static_cast<int>(s.opacity * 255)); // Pink
        p.setBrush(petalColor);
        
        // Draw a petal shape
        p.drawEllipse(QRectF(0, 0, s.size, s.size * 0.6));
        
        p.restore();
    }

    // 2. Draw Content Box Background (Glassmorphism / Sci-fi Panel)
    if (contentBox->isVisible()) {
        QRect boxRect = contentBox->geometry();
        
        QPainterPath path;
        path.addRoundedRect(boxRect, 40, 40);

        // Glass Fill
        QColor glassColor(20, 25, 40, 210);
        p.setBrush(glassColor);
        p.setPen(Qt::NoPen);
        p.drawPath(path);

        // Glowing Border
        QLinearGradient borderGrad(boxRect.topLeft(), boxRect.bottomRight());
        double borderHue = std::fmod(bgHue * 2.0, 360.0);
        borderGrad.setColorAt(0.0, QColor::fromHsvF(borderHue / 360.0, 1.0, 1.0, 0.8));
        borderGrad.setColorAt(0.5, QColor::fromHsvF(std::fmod(borderHue + 120.0, 360.0) / 360.0, 1.0, 1.0, 0.3));
        borderGrad.setColorAt(1.0, QColor::fromHsvF(std::fmod(borderHue + 240.0, 360.0) / 360.0, 1.0, 1.0, 0.8));
        
        QPen borderPen(borderGrad, 2);
        p.setPen(borderPen);
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);

        // 3. Draw Title with Flowing Light (鎏光) Effect
        QString titleText = "关于宝石迷阵";
        // Try artistic fonts: STXingkai (Calligraphy) or fallback
        QFont titleFont("STXingkai", 64, QFont::Bold);
        titleFont.setStyleStrategy(QFont::PreferAntialias);
        titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 6);
        p.setFont(titleFont);
        
        QFontMetrics fm(titleFont);
        int textWidth = fm.horizontalAdvance(titleText);
        int textHeight = fm.ascent();
        
        // Position: Centered horizontally, in the top margin area
        int textX = boxRect.center().x() - textWidth / 2;
        int textY = boxRect.top() + 90; 

        // 3.1 Draw Shadow
        p.setPen(QColor(0, 0, 0, 160));
        p.drawText(textX + 4, textY + 4, titleText);

        // 3.2 Draw Flowing Gradient Text
        // Create a gradient that moves with time
        QLinearGradient flowGrad(0, 0, 400, 0); // 400px wide repeating pattern
        flowGrad.setSpread(QGradient::RepeatSpread);
        
        // Gold/Gilded Theme Colors
        QColor goldDark(218, 165, 32);   // Goldenrod
        QColor goldBright(255, 215, 0);  // Gold
        QColor shine(255, 255, 255);     // White Shine
        
        flowGrad.setColorAt(0.0, goldDark);
        flowGrad.setColorAt(0.4, goldBright);
        flowGrad.setColorAt(0.5, shine);
        flowGrad.setColorAt(0.6, goldBright);
        flowGrad.setColorAt(1.0, goldDark);
        
        // Animate the gradient transform
        QTransform brushTrans;
        // Move the gradient pattern based on time
        brushTrans.translate(textX + std::fmod(time * 150.0, 400.0), 0); 
        
        QBrush textBrush(flowGrad);
        textBrush.setTransform(brushTrans);
        
        p.setPen(QPen(textBrush, 0));
        p.drawText(textX, textY, titleText);
        
        // Decorative corners (Tech look)
        p.setPen(QPen(QColor(255, 255, 255, 100), 4));
        int cornerLen = 30;
        // Top-Left
        p.drawLine(boxRect.left() + 20, boxRect.top() + 20, boxRect.left() + 20 + cornerLen, boxRect.top() + 20);
        p.drawLine(boxRect.left() + 20, boxRect.top() + 20, boxRect.left() + 20, boxRect.top() + 20 + cornerLen);
        // Bottom-Right
        p.drawLine(boxRect.right() - 20, boxRect.bottom() - 20, boxRect.right() - 20 - cornerLen, boxRect.bottom() - 20);
        p.drawLine(boxRect.right() - 20, boxRect.bottom() - 20, boxRect.right() - 20, boxRect.bottom() - 20 - cornerLen);
    }
}

void AboutWidget::updateSakuras() {
    // 1. Spawn new sakuras
    if (QRandomGenerator::global()->bounded(100) < 8) { // Increased spawn rate
        SakuraParticle s;
        // Spawn from left side or top
        if (QRandomGenerator::global()->bounded(2) == 0) {
            s.x = -50;
            s.y = QRandomGenerator::global()->bounded(height());
        } else {
            s.x = QRandomGenerator::global()->bounded(width());
            s.y = -50;
        }
        
        s.speedX = 4.0f + QRandomGenerator::global()->generateDouble() * 4.0f; // Faster wind
        s.speedY = 1.0f + QRandomGenerator::global()->generateDouble() * 2.0f; 
        s.rotation = QRandomGenerator::global()->bounded(360.0f);
        s.rotationSpeed = -3.0f + QRandomGenerator::global()->generateDouble() * 6.0f;
        s.size = 8.0f + QRandomGenerator::global()->generateDouble() * 8.0f;
        s.opacity = 0.6f + QRandomGenerator::global()->generateDouble() * 0.4f;
        
        sakuras.append(s);
    }

    // 2. Update positions
    for (int i = 0; i < sakuras.size(); ++i) {
        sakuras[i].x += sakuras[i].speedX;
        sakuras[i].y += sakuras[i].speedY;
        sakuras[i].rotation += sakuras[i].rotationSpeed;
        
        // Add some sine wave motion to Y
        sakuras[i].y += std::sin(time + sakuras[i].x * 0.01f) * 0.5f;

        // 3. Remove if out of bounds
        if (sakuras[i].x > width() + 50 || sakuras[i].y > height() + 50) {
            sakuras.removeAt(i);
            i--;
        }
    }
}
