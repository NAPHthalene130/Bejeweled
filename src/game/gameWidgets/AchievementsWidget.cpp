#include "AchievementsWidget.h"
#include "../GameWindow.h"
#include "../../utils/BackgroundManager.h"
#include "../../utils/ResourceUtils.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QFont>
#include <QMessageBox>
#include <QTimer>
#include <QFileInfo>
#include <QPixmap>
#include <QStackedLayout>
#include <QSizePolicy>
#include <QUrl>
#include <QVector3D>
#include <QGraphicsDropShadowEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QShowEvent>
#include <QHideEvent>
#include <QPainterPath>
#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>
#include <Qt3DExtras/qplanemesh.h>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qtextureimage.h>
#include <Qt3DExtras/qtexturematerial.h>
#include <Qt3DRender/qcamera.h>
#include <QWidget>

// Small widget to represent a single achievement
class AchievementItem : public QFrame {
    Q_OBJECT
public:
    AchievementItem(const AchievementData& a, QWidget* parent = nullptr) : QFrame(parent), data(a), flowPos(0.0) {
        setFrameShape(QFrame::StyledPanel);
        setLineWidth(1);
        setContentsMargins(10,10,10,10);
        // 紧凑卡片尺寸，确保6张卡片完美显示在一屏内
        setMinimumSize(280, 110);
        setMaximumSize(400, 160);
        setFixedSize(320, 130);
        // subtle drop shadow for depth
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(14);
        shadow->setOffset(0, 4);
        shadow->setColor(QColor(0,0,0,100));
        setGraphicsEffect(shadow);
        QVBoxLayout* l = new QVBoxLayout(this);
        titleLabel = new QLabel(data.getTitle(), this);
        QFont ft = titleLabel->font(); ft.setBold(true); ft.setPointSize(12); titleLabel->setFont(ft);
        descLabel = new QLabel(data.getDescription(), this);
        descLabel->setWordWrap(true);
        QFont df = descLabel->font(); df.setPointSize(9); descLabel->setFont(df);
        statusLabel = new QLabel(this);
        statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        if (data.isUnlocked()) {
            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, [this]() {
                flowPos += 0.02;
                if (flowPos > 1.2) flowPos = -0.2;
                update();
            });
            timer->start(30);
        } else {
            timer = nullptr;
        }

        QHBoxLayout* top = new QHBoxLayout();
        top->addWidget(titleLabel);
        top->addWidget(statusLabel, 0, Qt::AlignRight);
        l->addLayout(top);
        l->addSpacing(6);
        l->addWidget(descLabel);
    }

protected:
    void paintEvent(QPaintEvent* ev) override {
        QFrame::paintEvent(ev);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QColor base, gold(255, 215, 80);
        bool unlocked = data.isUnlocked();
        switch (data.getDifficulty()) {
            case AchievementData::Difficulty::Easy: base = unlocked ? QColor(74, 144, 226) : QColor(120,120,120); break;
            case AchievementData::Difficulty::Medium: base = unlocked ? QColor(255, 140, 0) : QColor(120,120,120); break;
            case AchievementData::Difficulty::Hard: base = unlocked ? QColor(209, 73, 73) : QColor(120,120,120); break;
            case AchievementData::Difficulty::Ultimate: base = unlocked ? QColor(112, 107, 206) : QColor(120,120,120); break;
        }
        QLinearGradient grad(rect().topLeft(), rect().bottomRight());
        grad.setColorAt(0.0, base.lighter(unlocked ? 115 : 105));
        grad.setColorAt(1.0, base.darker(unlocked ? 115 : 105));
        p.setBrush(grad);
        p.setPen(Qt::NoPen);
        int radius = 16;
        QRect r = rect().adjusted(0,0,0,0);
        p.drawRoundedRect(r, radius, radius);
        // subtle inner stroke for polish
        QPen stroke(QColor(255,255,255,30));
        stroke.setWidth(1);
        p.setPen(stroke);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(r.adjusted(1,1,-1,-1), radius, radius);
        if (unlocked) {
            QLinearGradient flow(rect().left() + flowPos * rect().width(), rect().top(), rect().left() + (flowPos+0.3) * rect().width(), rect().bottom());
            flow.setColorAt(0.0, QColor(gold.red(), gold.green(), gold.blue(), 0));
            flow.setColorAt(0.3, QColor(gold.red(), gold.green(), gold.blue(), 180));
            flow.setColorAt(0.7, QColor(gold.red(), gold.green(), gold.blue(), 0));
            p.setBrush(flow);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(r, radius, radius);
            // subtle gloss at top
            QLinearGradient gloss(r.topLeft(), r.bottomLeft());
            gloss.setColorAt(0.0, QColor(255,255,255,80));
            gloss.setColorAt(0.4, QColor(255,255,255,18));
            gloss.setColorAt(1.0, QColor(255,255,255,0));
            p.setBrush(gloss);
            p.drawRoundedRect(QRect(r.left(), r.top(), r.width(), r.height()/3), radius, radius);
            // 右下角显示解锁时间
            QDateTime dt = data.getCompletedAt();
            if (dt.isValid()) {
                QString t = dt.toString("yyyy-MM-dd HH:mm:ss");
                QFont f = font();
                f.setPointSize(9);
                p.setFont(f);
                p.setPen(QColor(60,60,60));
                p.drawText(r.adjusted(0,0,-12,-8), Qt::AlignRight | Qt::AlignBottom, t);
            }
        }
    }
    void mousePressEvent(QMouseEvent* ev) override {
        Q_UNUSED(ev)
        // visual feedback
        QString prev = styleSheet();
        setStyleSheet(prev + "border: 2px solid rgba(255,255,255,0.15);");
        QTimer::singleShot(150, this, [this, prev]() { setStyleSheet(prev); });
        // 不再弹窗
    }
private:
    AchievementData data;
    QLabel* titleLabel;
    QLabel* descLabel;
    QLabel* statusLabel;
    QTimer* timer;
    qreal flowPos;
};

#include "AchievementsWidget.moc"

AchievementsWidget::AchievementsWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    // 背景装饰层作为子 widget，放在最底层
    AchievementsBackgroundDecoration* deco = new AchievementsBackgroundDecoration(this);
    deco->setContentMargin(28);
    deco->lower(); // 确保在最底层
    
    // 初始化背景音乐播放器
    bgMusic = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    bgMusic->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.5); // 音量 50%
    
    // 加载背景音乐文件
    QString musicPath = QString::fromStdString(ResourceUtils::getPath("sounds/ambient.ogg"));
    qDebug() << "Music path:" << musicPath << "exists:" << QFile::exists(musicPath);
    if (QFile::exists(musicPath)) {
        bgMusic->setSource(QUrl::fromLocalFile(musicPath));
        bgMusic->setLoops(QMediaPlayer::Infinite); // 循环播放
    }
    
    // 监听播放错误
    connect(bgMusic, &QMediaPlayer::errorOccurred, this, [](QMediaPlayer::Error error, const QString &errorString) {
        qDebug() << "Media error:" << error << errorString;
    });
    connect(bgMusic, &QMediaPlayer::playbackStateChanged, this, [](QMediaPlayer::PlaybackState state) {
        qDebug() << "Playback state:" << state;
    });

    // 主布局直接设置在 this 上
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28,28,28,28);
    mainLayout->setSpacing(18);

    // 顶部布局：返回按钮 + 标题
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    // 返回按钮
    backButton = new QPushButton("← 返回", this);
    backButton->setFixedSize(100, 40);
    QFont backFont = backButton->font(); backFont.setPointSize(11); backFont.setBold(true);
    backButton->setFont(backFont);
    backButton->setStyleSheet(R"(
        QPushButton {
            color: white;
            border-radius: 10px;
            border: 1px solid rgba(255,255,255,40);
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(80,80,120,180), stop:1 rgba(60,60,100,180));
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(100,100,150,200), stop:1 rgba(80,80,130,200));
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(60,60,90,200), stop:1 rgba(50,50,80,200));
        }
    )");
    QGraphicsDropShadowEffect* backShadow = new QGraphicsDropShadowEffect(backButton);
    backShadow->setBlurRadius(12);
    backShadow->setOffset(0, 3);
    backShadow->setColor(QColor(0,0,0,100));
    backButton->setGraphicsEffect(backShadow);
    connect(backButton, &QPushButton::clicked, this, &AchievementsWidget::onBackClicked);
    
    QLabel* header = new QLabel("成就", this);
    header->setStyleSheet("color: white;");
    QFont hf = header->font(); hf.setPointSize(20); hf.setBold(true); header->setFont(hf);
    
    topLayout->addWidget(backButton);
    topLayout->addStretch(1);
    topLayout->addWidget(header);
    topLayout->addStretch(1);
    topLayout->addSpacing(100); // 平衡右侧空间
    mainLayout->addLayout(topLayout);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    gridContainer = new QWidget();
    gridLayout = new QGridLayout(gridContainer);
    gridLayout->setSpacing(18);
    gridLayout->setContentsMargins(0,0,0,0);
    scroll->setWidget(gridContainer);
    mainLayout->addWidget(scroll, 1);

    // 透明滚动区，卡片自身不透明
    scroll->setStyleSheet("background: transparent;");
    gridContainer->setStyleSheet("background: transparent;");

    navLayout = new QHBoxLayout();
    prevButton = new QPushButton("上一页", this);
    nextButton = new QPushButton("下一页", this);
    pageLabel = new QLabel(this);

    // Larger, refined navigation buttons
    QSize navBtnSize(140, 48);
    prevButton->setFixedSize(navBtnSize);
    nextButton->setFixedSize(navBtnSize);
    QFont navFont = prevButton->font(); navFont.setPointSize(12); navFont.setBold(true);
    prevButton->setFont(navFont);
    nextButton->setFont(navFont);

    // Gradient, rounded style and hover/pressed visual
    const QString navStyle = R"(
        QPushButton{
            color: white;
            border-radius: 12px;
            border: 1px solid rgba(255,255,255,35);
            padding: 6px 12px;
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6EE7F9, stop:1 #2BB6E0);
        }
        QPushButton:hover{
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #8FEFFB, stop:1 #3FC0EB);
        }
        QPushButton:pressed{
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #3FC0EB, stop:1 #2AA7CF);
        }
    )";
    prevButton->setStyleSheet(navStyle);
    nextButton->setStyleSheet(navStyle);

    // subtle shadow for buttons
    QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(prevButton);
    pShadow->setBlurRadius(20);
    pShadow->setOffset(0,6);
    pShadow->setColor(QColor(0,0,0,120));
    prevButton->setGraphicsEffect(pShadow);
    QGraphicsDropShadowEffect* nShadow = new QGraphicsDropShadowEffect(nextButton);
    nShadow->setBlurRadius(20);
    nShadow->setOffset(0,6);
    nShadow->setColor(QColor(0,0,0,120));
    nextButton->setGraphicsEffect(nShadow);

    // Page label polish
    QFont plf = pageLabel->font(); plf.setPointSize(12); pageLabel->setFont(plf);
    pageLabel->setAlignment(Qt::AlignCenter);

    navLayout->addWidget(prevButton);
    navLayout->addStretch(1);
    navLayout->addWidget(pageLabel);
    navLayout->addStretch(1);
    navLayout->addWidget(nextButton);
    navLayout->setSpacing(24);
    navLayout->setContentsMargins(0,12,0,12);
    mainLayout->addLayout(navLayout);

    // 页码标签样式
    pageLabel->setStyleSheet("color: white;");

    connect(prevButton, &QPushButton::clicked, this, &AchievementsWidget::onPrevPage);
    connect(nextButton, &QPushButton::clicked, this, &AchievementsWidget::onNextPage);

    refresh();
}
void AchievementsWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // 让装饰层跟随窗口大小
    for (QObject* child : children()) {
        AchievementsBackgroundDecoration* deco = qobject_cast<AchievementsBackgroundDecoration*>(child);
        if (deco) {
            deco->setGeometry(rect());
            deco->lower();
        }
    }
}

void AchievementsWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // 进入成就页面时播放背景音乐
    if (bgMusic && bgMusic->playbackState() != QMediaPlayer::PlayingState) {
        bgMusic->play();
    }
}

void AchievementsWidget::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    // 离开成就页面时停止背景音乐
    if (bgMusic) {
        bgMusic->stop();
    }
}

void AchievementsWidget::onPrevPage() {
    if (currentPage > 0) {
        currentPage--;
        refresh();
    }
}

void AchievementsWidget::onNextPage() {
    int total = (int)gameWindow->getAchievements().size();
    int pages = (total + itemsPerPage - 1) / itemsPerPage;
    if (currentPage + 1 < pages) {
        currentPage++;
        refresh();
    }
}

void AchievementsWidget::onBackClicked() {
    emit backToMenu();
}

void AchievementsWidget::updateView() {
    // reset to first page when updating view
    currentPage = 0;
    refresh();
}

void AchievementsWidget::refresh() {
    // clear grid
    QLayoutItem* child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    const auto& list = gameWindow->getAchievements();
    int total = (int)list.size();
    int pages = qMax(1, (total + itemsPerPage - 1) / itemsPerPage);

    int start = currentPage * itemsPerPage;
    int end = qMin(total, start + itemsPerPage);

    int colCount = 3; // 3 columns x 2 rows = 6 cards per page
    int row = 0, col = 0;
    for (int idx = 0; idx < end - start; ++idx) {
        const AchievementData& a = list[start + idx];
        AchievementItem* item = new AchievementItem(a, gridContainer);
        row = idx / colCount;
        col = idx % colCount;
        gridLayout->addWidget(item, row, col);
    }

    pageLabel->setText(QString("%1 / %2 页").arg(currentPage+1).arg(pages));

    prevButton->setEnabled(currentPage > 0);
    nextButton->setEnabled(currentPage + 1 < pages);
}

// 移除旧的 resizeEvent 和 updateBackground，避免重定义

#include <random>
#include <cmath>
#include <QMouseEvent>

AchievementsBackgroundDecoration::AchievementsBackgroundDecoration(QWidget* parent)
    : QWidget(parent) {
    // 不再设置全局鼠标穿透，改为在 mouseMoveEvent 中手动处理
    setAttribute(Qt::WA_TranslucentBackground);
    userBg = QPixmap();
    QString bgPath = QString::fromStdString(
        ResourceUtils::getPath(BackgroundManager::instance().getAchievementBackground())
    );
    if (QFile::exists(bgPath)) {
        bgImage.load(bgPath);
    }
    setMouseTracking(true);
    
    // 初始化动态星星粒子
    initParticles();
    
    // 启动动画定时器
    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &AchievementsBackgroundDecoration::updateAnimation);
    animTimer->start(33); // ~30fps
}

void AchievementsBackgroundDecoration::mouseMoveEvent(QMouseEvent* event) {
    // 检测鼠标是否在星星区域
    bool wasHover = starShy;
    starShy = starRectCache.contains(event->pos());
    
    if (wasHover != starShy) {
        update(); // 状态变化时重绘
    }
    
    // 如果不在星星上，让事件传递给下层 widget
    if (!starShy) {
        event->ignore();
    } else {
        event->accept();
    }
}

void AchievementsBackgroundDecoration::leaveEvent(QEvent* event) {
    if (starShy) {
        starShy = false;
        update();
    }
    QWidget::leaveEvent(event);
}

void AchievementsBackgroundDecoration::initParticles() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> sizeDist(1.5f, 4.0f);
    std::uniform_real_distribution<float> phaseDist(0.0f, 6.28f);
    std::uniform_real_distribution<float> speedDist(0.5f, 2.0f);
    std::uniform_real_distribution<float> driftDist(-0.0003f, 0.0003f);
    
    particles.clear();
    int numParticles = 60; // 星星数量
    for (int i = 0; i < numParticles; ++i) {
        StarParticle p;
        p.x = posDist(gen);
        p.y = posDist(gen);
        p.size = sizeDist(gen);
        p.alpha = posDist(gen);
        p.twinklePhase = phaseDist(gen);
        p.twinkleSpeed = speedDist(gen);
        p.driftX = driftDist(gen);
        p.driftY = driftDist(gen);
        particles.push_back(p);
    }
}

void AchievementsBackgroundDecoration::updateAnimation() {
    animTime += 0.033f;
    
    // 更新每个粒子
    for (auto& p : particles) {
        // 位置飘动
        p.x += p.driftX;
        p.y += p.driftY;
        
        // 边界环绕
        if (p.x < 0) p.x = 1.0f;
        if (p.x > 1) p.x = 0.0f;
        if (p.y < 0) p.y = 1.0f;
        if (p.y > 1) p.y = 0.0f;
    }
    
    update(); // 触发重绘
}

void AchievementsBackgroundDecoration::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::Antialiasing);

    // 1. 先绘制背景图片（整体动画效果）
    if (!bgImage.isNull()) {
        QPixmap scaled = bgImage.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // 居中裁剪偏移
        int offsetX = (scaled.width() - width()) / 2;
        int offsetY = (scaled.height() - height()) / 2;
        
        // 区域划分：
        // 云朵区域：画面上方25%有云飘动效果
        int cloudEndY = static_cast<int>(height() * 0.25f);
        // 云过渡区域：25%-35%之间平滑过渡
        int cloudTransitionEndY = static_cast<int>(height() * 0.35f);
        // 35%-80%：静止区域（椰树、房子、沙滩主体）
        // 海浪区域：80%-98%（只有海水部分有波动）
        int waveStartY = static_cast<int>(height() * 0.80f);
        int waveEndY = static_cast<int>(height() * 0.98f);
        
        float globalDrift = animTime * 8.0f; // 云朵向右飘动速度
        
        // 逐行绘制整个画面
        for (int row = 0; row < height(); ++row) {
            float totalOffset = 0.0f;
            bool needWrap = false; // 是否需要循环处理（只有云区域需要）
            
            if (row < cloudEndY) {
                // 纯云区域：完整的飘动效果
                float cloudHeight = 1.0f - (float)row / cloudEndY; // 越高飘得越快
                totalOffset = std::fmod(globalDrift * (0.5f + cloudHeight * 0.5f), (float)scaled.width());
                totalOffset += std::sin(animTime * 0.5f + row * 0.008f) * 3.0f * cloudHeight;
                needWrap = true;
            } else if (row < cloudTransitionEndY) {
                // 云过渡区域：云飘动效果逐渐减弱到0
                float transitionProgress = (float)(row - cloudEndY) / (cloudTransitionEndY - cloudEndY);
                float fadeOut = 1.0f - transitionProgress;
                fadeOut = fadeOut * fadeOut;
                totalOffset = std::fmod(globalDrift * 0.5f * fadeOut, (float)scaled.width());
                totalOffset += std::sin(animTime * 0.5f + row * 0.008f) * 3.0f * fadeOut;
                needWrap = (totalOffset > 1.0f);
            } else if (row >= waveStartY && row < waveEndY) {
                // 海浪效果：上部海面轻柔波动，越靠近沙滩波动越强
                float waveProgress = (float)(row - waveStartY) / (waveEndY - waveStartY);
                // 从上到下逐渐增强：上部0.3，底部1.0
                float waveIntensity = 0.3f + waveProgress * 0.7f;
                // 多层波浪叠加
                float wave1 = std::sin(animTime * 1.8f + row * 0.04f) * 8.0f;
                float wave2 = std::sin(animTime * 2.5f + row * 0.06f) * 5.0f;
                float wave3 = std::sin(animTime * 1.0f + row * 0.025f) * 6.0f;
                totalOffset = (wave1 + wave2 + wave3) * waveIntensity;
            }
            // 其他区域（cloudTransitionEndY到waveStartY，以及waveEndY以下）：静止
            
            int srcX = offsetX + static_cast<int>(totalOffset);
            
            if (needWrap) {
                // 云区域需要循环处理
                srcX = ((srcX % scaled.width()) + scaled.width()) % scaled.width();
                
                int remainWidth = scaled.width() - srcX;
                if (remainWidth >= width()) {
                    p.drawPixmap(0, row, scaled, srcX, offsetY + row, width(), 1);
                } else {
                    p.drawPixmap(0, row, scaled, srcX, offsetY + row, remainWidth, 1);
                    p.drawPixmap(remainWidth, row, scaled, 0, offsetY + row, width() - remainWidth, 1);
                }
            } else {
                // 非云区域：限制偏移范围，不循环
                srcX = qBound(0, srcX, scaled.width() - width());
                p.drawPixmap(0, row, scaled, srcX, offsetY + row, width(), 1);
            }
        }
    } else {
        // 如果没有背景图片，使用蓝紫渐变背景
        QLinearGradient grad(rect().topLeft(), rect().bottomRight());
        grad.setColorAt(0.0, QColor(30, 20, 60, 255));
        grad.setColorAt(0.5, QColor(40, 30, 80, 255));
        grad.setColorAt(1.0, QColor(60, 40, 120, 255));
        p.fillRect(rect(), grad);
    }

    // 2. 绘制动态闪烁星星粒子
    for (const auto& star : particles) {
        float twinkle = 0.5f + 0.5f * std::sin(animTime * star.twinkleSpeed + star.twinklePhase);
        int alpha = static_cast<int>(twinkle * 255);
        int px = static_cast<int>(star.x * width());
        int py = static_cast<int>(star.y * height());
        int sz = static_cast<int>(star.size * (0.8f + 0.4f * twinkle));
        
        // 星星发光
        QRadialGradient starGlow(px, py, sz * 3);
        starGlow.setColorAt(0.0, QColor(255, 255, 255, alpha));
        starGlow.setColorAt(0.3, QColor(200, 220, 255, alpha / 2));
        starGlow.setColorAt(1.0, QColor(150, 180, 255, 0));
        p.setBrush(starGlow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPoint(px, py), sz * 3, sz * 3);
        
        // 星星核心
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPoint(px, py), sz, sz);
    }

    // 星星人偶参数
    int starSize = 64;
    int margin = 40;
    QPoint starCenter(width() - starSize/2 - margin, starSize/2 + margin); // 右上角
    QRect starRect(starCenter.x() - starSize/2, starCenter.y() - starSize/2, starSize, starSize);
    // 只检测星星本体，不检测发光区域，避免遮挡按钮
    starRectCache = starRect.adjusted(8, 8, -8, -8);

    // 发光效果
    if (!starShy) {
        double glowRadius = starSize * 1.2; // 半径1.2倍
        QRadialGradient glow(QPointF(starCenter), glowRadius);
        glow.setColorAt(0.0, QColor(255,255,200,255)); // 更亮
        glow.setColorAt(0.3, QColor(255,255,200,180));
        glow.setColorAt(0.7, QColor(255,255,180,80));
        glow.setColorAt(1.0, QColor(255,255,180,0));
        p.setBrush(glow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(starCenter), glowRadius, glowRadius);
    }

    // 画星星形状
    QPolygon star;
    for (int i = 0; i < 5; ++i) {
        double angle = i * 2 * M_PI / 5 - M_PI/2;
        double x = starCenter.x() + std::cos(angle) * starSize/2;
        double y = starCenter.y() + std::sin(angle) * starSize/2;
        star << QPoint(int(x), int(y));
        angle += M_PI / 5;
        x = starCenter.x() + std::cos(angle) * starSize/5;
        y = starCenter.y() + std::sin(angle) * starSize/5;
        star << QPoint(int(x), int(y));
    }
    p.setBrush(QColor(255, 255, 180));
    p.setPen(QPen(QColor(220, 200, 80), 2));
    p.drawPolygon(star);

    // 画人偶脸部
    QRect faceRect(starCenter.x()-14, starCenter.y()-8, 28, 20);
    p.setBrush(QColor(255,255,255,230));
    p.setPen(Qt::NoPen);
    p.drawEllipse(faceRect);
    // 眼睛
    p.setBrush(Qt::black);
    if (!starShy) {
        p.drawEllipse(QRect(starCenter.x()-7, starCenter.y()-2, 4, 4));
        p.drawEllipse(QRect(starCenter.x()+3, starCenter.y()-2, 4, 4));
        // 微笑
        QPainterPath smile;
        smile.moveTo(starCenter.x()-4, starCenter.y()+6);
        smile.quadTo(starCenter.x(), starCenter.y()+10, starCenter.x()+4, starCenter.y()+6);
        p.setPen(QPen(Qt::black, 1.5));
        p.drawPath(smile);
    } else {
        // 害羞表情：眼睛变弯，脸颊红晕
        QPainterPath shyEyeL, shyEyeR;
        shyEyeL.moveTo(starCenter.x()-7, starCenter.y());
        shyEyeL.quadTo(starCenter.x()-5, starCenter.y()+2, starCenter.x()-3, starCenter.y());
        shyEyeR.moveTo(starCenter.x()+3, starCenter.y());
        shyEyeR.quadTo(starCenter.x()+5, starCenter.y()+2, starCenter.x()+7, starCenter.y());
        p.setPen(QPen(Qt::black, 1.2));
        p.drawPath(shyEyeL);
        p.drawPath(shyEyeR);
        // 害羞嘴
        QPainterPath shyMouth;
        shyMouth.moveTo(starCenter.x()-3, starCenter.y()+8);
        shyMouth.quadTo(starCenter.x(), starCenter.y()+6, starCenter.x()+3, starCenter.y()+8);
        p.setPen(QPen(Qt::black, 1.2));
        p.drawPath(shyMouth);
        // 脸颊红晕
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255,120,120,120));
        p.drawEllipse(QRect(starCenter.x()-11, starCenter.y()+4, 7, 4));
        p.drawEllipse(QRect(starCenter.x()+4, starCenter.y()+4, 7, 4));
    }
}



