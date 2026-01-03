#include "RankListWidget.h"
#include "../GameWindow.h"
#include "../../utils/ResourceUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QFont>
#include <QFile>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <QUrl>
#include <algorithm>
#include <cmath>

RankListWidget::RankListWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    // 加载背景图片 - 尝试多个可能的路径
    QStringList possiblePaths = {
        QString::fromStdString(ResourceUtils::getPath("rank_bg.png")),
        QCoreApplication::applicationDirPath() + "/assets/rank_bg.png",
        QCoreApplication::applicationDirPath() + "/../assets/rank_bg.png",
        QCoreApplication::applicationDirPath() + "/../../assets/rank_bg.png",
        "D:/Bejeweled/assets/rank_bg.png",
        "D:/Bejeweled/build/assets/rank_bg.png"
    };
    
    for (const QString& path : possiblePaths) {
        qDebug() << "Trying rank_bg path:" << path << "exists:" << QFile::exists(path);
        if (QFile::exists(path)) {
            if (bgImage.load(path)) {
                qDebug() << "Successfully loaded rank_bg from:" << path;
                break;
            }
        }
    }
    
    if (bgImage.isNull()) {
        qDebug() << "Failed to load rank_bg.png from any path";
    }
    
    setMinimumSize(1280, 720);

    setupUI();
    
    refreshDisplay();
    
    // 初始化鎏金动画定时器
    goldenAnimTimer = new QTimer(this);
    connect(goldenAnimTimer, &QTimer::timeout, this, &RankListWidget::updateGoldenAnimation);
    goldenAnimTimer->start(50);  // 20fps动画
    
    // 初始化背景动画定时器
    bgAnimTimer = new QTimer(this);
    connect(bgAnimTimer, &QTimer::timeout, this, &RankListWidget::updateBackgroundAnimation);
    bgAnimTimer->start(30);  // 约33fps动画
    
    // 初始化浮动粒子
    std::srand(static_cast<unsigned>(time(nullptr)));
    for (int i = 0; i < 30; ++i) {
        Particle p;
        p.x = std::rand() % 1600;
        p.y = std::rand() % 1000;
        p.speedX = (std::rand() % 100 - 50) / 100.0f;  // -0.5 到 0.5
        p.speedY = (std::rand() % 100 - 70) / 100.0f;  // 主要向上飘
        p.size = 2 + std::rand() % 6;
        p.alpha = 50 + std::rand() % 150;
        p.phase = (std::rand() % 628) / 100.0f;  // 随机初始相位
        particles.push_back(p);
    }
    
    // 初始化海鸥
    for (int i = 0; i < 5; ++i) {
        Seagull s;
        s.x = std::rand() % 1600;
        s.y = 50 + std::rand() % 200;  // 在天空上方区域
        s.speed = 1.0f + (std::rand() % 100) / 100.0f;  // 1.0 到 2.0
        s.wingPhase = (std::rand() % 628) / 100.0f;
        s.size = 15 + std::rand() % 10;  // 15-25
        seagulls.push_back(s);
    }
    
    // 初始化排行榜背景音乐播放器
    bgmPlayer = new QMediaPlayer(this);
    bgmAudioOutput = new QAudioOutput(this);
    bgmPlayer->setAudioOutput(bgmAudioOutput);
    bgmAudioOutput->setVolume(0.5f);  // 设置音量为50%
    bgmPlayer->setLoops(QMediaPlayer::Infinite);  // 循环播放
    
    // 加载音乐文件
    QStringList bgmPaths = {
        QCoreApplication::applicationDirPath() + "/resources/sounds/rank_bgm.mp3",
        "D:/Bejeweled/build/resources/sounds/rank_bgm.mp3"
    };
    
    for (const QString& path : bgmPaths) {
        if (QFile::exists(path)) {
            bgmPlayer->setSource(QUrl::fromLocalFile(path));
            qDebug() << "Rank BGM loaded from:" << path;
            break;
        }
    }
}

void RankListWidget::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);
    
    // 顶部布局：返回按钮 + 标题
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    // 返回按钮
    backButton = new QPushButton("← 返回", this);
    backButton->setFixedSize(100, 40);
    QFont backFont = backButton->font();
    backFont.setPointSize(11);
    backFont.setBold(true);
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
    backShadow->setColor(QColor(0, 0, 0, 100));
    backButton->setGraphicsEffect(backShadow);
    connect(backButton, &QPushButton::clicked, this, &RankListWidget::onBackClicked);
    
    // 标题
    titleLabel = new QLabel("排行榜", this);
    titleLabel->setStyleSheet("color: white; background: transparent;");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    
    topLayout->addWidget(backButton);
    topLayout->addStretch(1);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch(1);
    topLayout->addSpacing(100);
    mainLayout->addLayout(topLayout);
    
    // Tab 控件
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid rgba(255, 255, 255, 30);
            border-radius: 12px;
            background: rgba(255, 255, 255, 15);
            top: -1px;
        }
        QTabBar::tab {
            background: rgba(60, 60, 80, 180);
            color: rgba(255, 255, 255, 180);
            border: 1px solid rgba(255, 255, 255, 30);
            border-bottom: none;
            border-top-left-radius: 10px;
            border-top-right-radius: 10px;
            padding: 12px 30px;
            margin-right: 4px;
            font-size: 14px;
            font-weight: bold;
        }
        QTabBar::tab:selected {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 rgba(100, 180, 255, 200), stop:1 rgba(60, 140, 220, 200));
            color: white;
        }
        QTabBar::tab:hover:!selected {
            background: rgba(80, 80, 110, 200);
            color: white;
        }
    )");
    
    // 创建三个表格
    normalModeTable = new QTableWidget(this);
    rotateModeTable = new QTableWidget(this);
    multiplayerTable = new QTableWidget(this);
    
    setupTab(normalModeTable, {"排名", "用户", "时间(秒)"});
    setupTab(rotateModeTable, {"排名", "用户", "分数"});
    setupTab(multiplayerTable, {"排名", "用户", "分数"});
    
    tabWidget->addTab(normalModeTable, "🎮 普通模式");
    tabWidget->addTab(rotateModeTable, "🌀 旋风模式");
    tabWidget->addTab(multiplayerTable, "⚔️ 多人对战");
    
    mainLayout->addWidget(tabWidget, 1);
    
    // 底部说明
    QLabel* infoLabel = new QLabel("* 排行榜记录您的历史最佳前10名成绩", this);
    infoLabel->setStyleSheet("color: rgba(255, 255, 255, 150); background: transparent;");
    infoLabel->setAlignment(Qt::AlignCenter);
    QFont infoFont = infoLabel->font();
    infoFont.setPointSize(10);
    infoLabel->setFont(infoFont);
    mainLayout->addWidget(infoLabel);
}

void RankListWidget::setupTab(QTableWidget* table, const QStringList& headers) {
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    
    table->setRowCount(10);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setShowGrid(false);
    table->setAlternatingRowColors(false); // 禁用交替行颜色，以便显示自定义背景
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // 设置行高
    table->verticalHeader()->setDefaultSectionSize(60);
    
    // 表格样式
    table->setStyleSheet(R"(
        QTableWidget {
            background: transparent;
            border: none;
            color: white;
            font-size: 14px;
        }
        QTableWidget::item {
            padding: 10px;
            border-bottom: 1px solid rgba(255, 255, 255, 20);
        }
        QTableWidget::item:selected {
            background: rgba(100, 180, 255, 100);
        }
        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(80, 100, 140, 200), stop:1 rgba(60, 80, 120, 200));
            color: white;
            padding: 12px;
            border: none;
            border-bottom: 2px solid rgba(100, 180, 255, 150);
            font-size: 14px;
            font-weight: bold;
        }
    )");
}

void RankListWidget::updateTable(QTableWidget* table, const std::vector<RankRecord>& records) {
    table->clearContents();
    
    // 排名图标/奖牌
    QStringList rankIcons = {"🥇", "🥈", "🥉", "4", "5", "6", "7", "8", "9", "10"};
    
    // 非前三名的颜色
    QColor dimColor(200, 210, 230);  // 亮白蓝色
    
    for (int i = 0; i < 10; ++i) {
        if (i < (int)records.size()) {
            const RankRecord& rec = records[i];
            
            // 排名
            QTableWidgetItem* rankItem = new QTableWidgetItem(rankIcons[i]);
            rankItem->setTextAlignment(Qt::AlignCenter);
            rankItem->setFont(QFont("Segoe UI Emoji", 16));
            if (i < 3) applyGoldenGlowEffect(rankItem, i);
            else rankItem->setForeground(dimColor);
            table->setItem(i, 0, rankItem);

            // 用户ID
            QTableWidgetItem* idItem = new QTableWidgetItem(QString::fromStdString(rec.id));
            idItem->setTextAlignment(Qt::AlignCenter);
            QFont idFont = idItem->font();
            idFont.setPointSize(12);
            idItem->setFont(idFont);
            if (i < 3) applyGoldenGlowEffect(idItem, i);
            else idItem->setForeground(dimColor);
            table->setItem(i, 1, idItem);
            
            // 分数
            QTableWidgetItem* scoreItem = new QTableWidgetItem(QString::number(rec.score));
            scoreItem->setTextAlignment(Qt::AlignCenter);
            QFont scoreFont = scoreItem->font();
            scoreFont.setBold(true);
            scoreFont.setPointSize(14);
            scoreItem->setFont(scoreFont);
            if (i < 3) applyGoldenGlowEffect(scoreItem, i);
            else scoreItem->setForeground(dimColor);
            table->setItem(i, 2, scoreItem);
        } else {
            // 空行显示 "--"
            for (int j = 0; j < 3; ++j) {
                QTableWidgetItem* emptyItem = new QTableWidgetItem(j == 0 ? QString::number(i + 1) : "--");
                emptyItem->setTextAlignment(Qt::AlignCenter);
                emptyItem->setForeground(QColor(100, 100, 100));
                table->setItem(i, j, emptyItem);
            }
        }
    }
}

void RankListWidget::sortAndKeepTop10(std::vector<RankRecord>& records, bool ascending) {
    // 排序
    std::sort(records.begin(), records.end(), [ascending](const RankRecord& a, const RankRecord& b) {
        if (ascending) return a.score < b.score;
        return a.score > b.score;
    });
    // 只保留前10条
    if (records.size() > 10) {
        records.resize(10);
    }
}

void RankListWidget::applyGoldenGlowEffect(QTableWidgetItem* item, int rank) {
    // 将item添加到动画列表
    goldenItems.push_back(item);
    
    // 存储rank信息到item的data中
    item->setData(Qt::UserRole, rank);
    
    // 设置初始颜色
    QColor color = getAnimatedGoldColor(rank, goldenAnimPhase);
    item->setForeground(color);
    
    // 设置粗体字体
    QFont font = item->font();
    font.setBold(true);
    item->setFont(font);
}

QColor RankListWidget::getAnimatedGoldColor(int rank, float phase) {
    // 使用正弦波创建闪烁效果，不同排名有不同的相位偏移
    float offset = rank * 0.5f;
    float wave = 0.5f + 0.5f * std::sin(phase + offset);
    
    switch (rank) {
        case 0: {
            // 第一名 - 金色闪光：从深金色到亮金色
            int r = 200 + (int)(55 * wave);   // 200-255
            int g = 160 + (int)(95 * wave);   // 160-255 
            int b = (int)(100 * wave);        // 0-100
            return QColor(r, g, b);
        }
        case 1: {
            // 第二名 - 银色闪光：从灰银到亮白
            int base = 170 + (int)(85 * wave); // 170-255
            return QColor(base, base, std::min(255, base + 20));
        }
        case 2: {
            // 第三名 - 铜色闪光：从暗铜到亮铜
            int r = 180 + (int)(75 * wave);   // 180-255
            int g = 100 + (int)(80 * wave);   // 100-180
            int b = 50 + (int)(50 * wave);    // 50-100
            return QColor(r, g, b);
        }
        default:
            return QColor(255, 255, 255);
    }
}

void RankListWidget::updateGoldenAnimation() {
    // 更新动画相位
    goldenAnimPhase += 0.15f;
    if (goldenAnimPhase > 6.28f) {
        goldenAnimPhase -= 6.28f;
    }
    
    // 更新所有鎏金item的颜色
    for (QTableWidgetItem* item : goldenItems) {
        if (item) {
            int rank = item->data(Qt::UserRole).toInt();
            QColor color = getAnimatedGoldColor(rank, goldenAnimPhase);
            item->setForeground(color);
        }
    }
}

void RankListWidget::updateBackgroundAnimation() {
    // 更新背景动画相位
    bgAnimPhase += 0.05f;
    if (bgAnimPhase > 628.0f) {
        bgAnimPhase -= 628.0f;
    }
    
    // 更新粒子位置
    for (auto& particle : particles) {
        particle.x += particle.speedX;
        particle.y += particle.speedY;
        
        // 粒子超出边界时重新生成
        if (particle.y < -20) {
            particle.y = height() + 20;
            particle.x = std::rand() % width();
        }
        if (particle.x < -20) particle.x = width() + 20;
        if (particle.x > width() + 20) particle.x = -20;
    }
    
    // 更新海鸥位置
    for (auto& seagull : seagulls) {
        seagull.x += seagull.speed;
        seagull.wingPhase += 0.2f;  // 翅膀扇动速度
        
        // 海鸥飞出屏幕右侧时从左侧重新进入
        if (seagull.x > width() + 50) {
            seagull.x = -50;
            seagull.y = 50 + std::rand() % 200;
        }
    }
    
    // 触发重绘
    update();
}

void RankListWidget::setNormalModeRecords(const std::vector<std::pair<std::string, int>>& records) {
    normalModeRecords.clear();
    for (const auto& p : records) {
        normalModeRecords.emplace_back(p.first, p.second);
    }
    sortAndKeepTop10(normalModeRecords, true); // Ascending
    updateTable(normalModeTable, normalModeRecords);
}

void RankListWidget::setRotateModeRecords(const std::vector<std::pair<std::string, int>>& records) {
    rotateModeRecords.clear();
    for (const auto& p : records) {
        rotateModeRecords.emplace_back(p.first, p.second);
    }
    sortAndKeepTop10(rotateModeRecords, false); // Descending
    updateTable(rotateModeTable, rotateModeRecords);
}

void RankListWidget::setMultiplayerRecords(const std::vector<std::pair<std::string, int>>& records) {
    multiplayerRecords.clear();
    for (const auto& p : records) {
        multiplayerRecords.emplace_back(p.first, p.second);
    }
    sortAndKeepTop10(multiplayerRecords, false); // Descending
    updateTable(multiplayerTable, multiplayerRecords);
}

void RankListWidget::refreshDisplay() {
    goldenItems.clear();  // 清空以便重新收集
    updateTable(normalModeTable, normalModeRecords);
    updateTable(rotateModeTable, rotateModeRecords);
    updateTable(multiplayerTable, multiplayerRecords);
}

void RankListWidget::onBackClicked() {
    emit backToMenu();
}

void RankListWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    
    // 绘制背景图片（静态，不移动）
    if (!bgImage.isNull()) {
        QPixmap scaled = bgImage.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        int offsetX = (scaled.width() - width()) / 2;
        int offsetY = (scaled.height() - height()) / 2;
        p.drawPixmap(0, 0, scaled, offsetX, offsetY, width(), height());
        
        // 添加半透明遮罩层，确保排行榜内容清晰可见
        p.fillRect(rect(), QColor(0, 0, 0, 80));
    } else {
        // 如果没有背景图片，使用渐变背景
        QLinearGradient grad(rect().topLeft(), rect().bottomRight());
        grad.setColorAt(0.0, QColor(25, 25, 45));
        grad.setColorAt(0.5, QColor(35, 35, 65));
        grad.setColorAt(1.0, QColor(45, 30, 70));
        p.fillRect(rect(), grad);
    }
    
    // 绘制海鸥
    p.setPen(QPen(QColor(30, 30, 30), 2));
    for (const auto& seagull : seagulls) {
        // 翅膀扇动效果
        float wingAngle = std::sin(seagull.wingPhase) * 0.4f;  // 翅膀上下扇动
        
        float sz = seagull.size;
        float x = seagull.x;
        float y = seagull.y;
        
        // 绘制海鸥（简化的 M 形状）
        QPainterPath path;
        // 左翅膀
        path.moveTo(x - sz, y + sz * wingAngle);
        path.quadTo(x - sz * 0.5, y - sz * 0.3 + sz * wingAngle * 0.5, x, y);
        // 右翅膀
        path.quadTo(x + sz * 0.5, y - sz * 0.3 + sz * wingAngle * 0.5, x + sz, y + sz * wingAngle);
        
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(40, 40, 50), 2.5));
        p.drawPath(path);
        
        // 绘制身体小点
        p.setBrush(QColor(40, 40, 50));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(x, y), sz * 0.15, sz * 0.1);
    }
    
    // 绘制浮动粒子（发光效果）
    p.setPen(Qt::NoPen);
    for (const auto& particle : particles) {
        // 粒子呼吸效果
        float breathe = 0.7f + 0.3f * std::sin(bgAnimPhase + particle.phase);
        int alpha = static_cast<int>(particle.alpha * breathe);
        
        // 绘制光晕
        QRadialGradient glow(particle.x, particle.y, particle.size * 2);
        glow.setColorAt(0, QColor(255, 255, 255, alpha));
        glow.setColorAt(0.5, QColor(200, 220, 255, alpha / 2));
        glow.setColorAt(1, QColor(150, 180, 255, 0));
        p.setBrush(glow);
        p.drawEllipse(QPointF(particle.x, particle.y), particle.size * 2, particle.size * 2);
        
        // 绘制粒子核心
        p.setBrush(QColor(255, 255, 255, alpha));
        p.drawEllipse(QPointF(particle.x, particle.y), particle.size * 0.5, particle.size * 0.5);
    }
}

void RankListWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    update(); // 重绘背景
}

void RankListWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // 进入排行榜页面时播放背景音乐
    if (bgmPlayer) {
        bgmPlayer->play();
        qDebug() << "Playing rank BGM";
    }
}

void RankListWidget::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    // 离开排行榜页面时停止音乐
    if (bgmPlayer) {
        bgmPlayer->stop();
    }
}
