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
#include <QLinearGradient>
#include <QFont>
#include <QFile>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
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
    
    setupUI();
    
    // 添加一些示例数据
    addNormalModeRecord(15000, 180);
    addNormalModeRecord(12500, 150);
    addNormalModeRecord(18000, 200);
    addNormalModeRecord(9500, 120);
    addNormalModeRecord(21000, 250);
    
    addRotateModeRecord(8000, 60);
    addRotateModeRecord(12000, 90);
    addRotateModeRecord(6500, 45);
    
    addMultiplayerRecord(5000, 120, "玩家A", true);
    addMultiplayerRecord(4500, 100, "玩家B", false);
    addMultiplayerRecord(6200, 130, "玩家C", true);
    addMultiplayerRecord(3800, 90, "玩家D", true);
    
    refreshDisplay();
    
    // 初始化鎏金动画定时器
    goldenAnimTimer = new QTimer(this);
    connect(goldenAnimTimer, &QTimer::timeout, this, &RankListWidget::updateGoldenAnimation);
    goldenAnimTimer->start(50);  // 20fps动画
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
    
    setupTab(normalModeTable, false);
    setupTab(rotateModeTable, false);
    setupTab(multiplayerTable, true);
    
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

void RankListWidget::setupTab(QTableWidget* table, bool isMultiplayer) {
    if (isMultiplayer) {
        table->setColumnCount(6);
        table->setHorizontalHeaderLabels({"排名", "分数", "用时", "对手", "结果", "日期"});
    } else {
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({"排名", "分数", "用时", "日期"});
    }
    
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

void RankListWidget::updateTable(QTableWidget* table, const std::vector<RankRecord>& records, bool isMultiplayer) {
    table->clearContents();
    
    // 排名图标/奖牌
    QStringList rankIcons = {"🥇", "🥈", "🥉", "4", "5", "6", "7", "8", "9", "10"};
    
    // 非前三名的淡色
    QColor dimColor(120, 130, 150);  // 淡灰蓝色
    
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
            
            // 分数
            QTableWidgetItem* scoreItem = new QTableWidgetItem(QString::number(rec.score));
            scoreItem->setTextAlignment(Qt::AlignCenter);
            QFont scoreFont = scoreItem->font();
            scoreFont.setBold(true);
            scoreFont.setPointSize(14);
            scoreItem->setFont(scoreFont);
            if (i < 3) applyGoldenGlowEffect(scoreItem, i);
            else scoreItem->setForeground(dimColor);
            table->setItem(i, 1, scoreItem);
            
            // 用时
            QTableWidgetItem* timeItem = new QTableWidgetItem(formatDuration(rec.duration));
            timeItem->setTextAlignment(Qt::AlignCenter);
            if (i < 3) applyGoldenGlowEffect(timeItem, i);
            else timeItem->setForeground(dimColor);
            table->setItem(i, 2, timeItem);
            
            if (isMultiplayer) {
                // 对手
                QTableWidgetItem* oppItem = new QTableWidgetItem(rec.opponentName);
                oppItem->setTextAlignment(Qt::AlignCenter);
                if (i < 3) applyGoldenGlowEffect(oppItem, i);
                else oppItem->setForeground(dimColor);
                table->setItem(i, 3, oppItem);
                
                // 结果
                QTableWidgetItem* resultItem = new QTableWidgetItem(rec.isWin ? "🏆 胜利" : "💔 失败");
                resultItem->setTextAlignment(Qt::AlignCenter);
                if (i >= 3) resultItem->setForeground(rec.isWin ? QColor(80, 180, 80) : QColor(180, 80, 80));  // 淡化的胜负颜色
                else applyGoldenGlowEffect(resultItem, i);
                table->setItem(i, 4, resultItem);
                
                // 日期
                QTableWidgetItem* dateItem = new QTableWidgetItem(rec.playedAt.toString("MM-dd HH:mm"));
                dateItem->setTextAlignment(Qt::AlignCenter);
                if (i < 3) applyGoldenGlowEffect(dateItem, i);
                else dateItem->setForeground(QColor(100, 110, 130));  // 更淡的日期
                table->setItem(i, 5, dateItem);
            } else {
                // 日期
                QTableWidgetItem* dateItem = new QTableWidgetItem(rec.playedAt.toString("MM-dd HH:mm"));
                dateItem->setTextAlignment(Qt::AlignCenter);
                if (i < 3) applyGoldenGlowEffect(dateItem, i);
                else dateItem->setForeground(QColor(100, 110, 130));  // 更淡的日期
                table->setItem(i, 3, dateItem);
            }
        } else {
            // 空行显示 "--"
            int cols = isMultiplayer ? 6 : 4;
            for (int j = 0; j < cols; ++j) {
                QTableWidgetItem* emptyItem = new QTableWidgetItem(j == 0 ? QString::number(i + 1) : "--");
                emptyItem->setTextAlignment(Qt::AlignCenter);
                emptyItem->setForeground(QColor(100, 100, 100));
                table->setItem(i, j, emptyItem);
            }
        }
    }
}

void RankListWidget::sortAndKeepTop10(std::vector<RankRecord>& records) {
    // 按分数降序排序
    std::sort(records.begin(), records.end(), [](const RankRecord& a, const RankRecord& b) {
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

QString RankListWidget::formatDuration(int seconds) const {
    int mins = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}

void RankListWidget::addNormalModeRecord(int score, int duration) {
    RankRecord rec(score, duration, QDateTime::currentDateTime());
    normalModeRecords.push_back(rec);
    sortAndKeepTop10(normalModeRecords);
}

void RankListWidget::addRotateModeRecord(int score, int duration) {
    RankRecord rec(score, duration, QDateTime::currentDateTime());
    rotateModeRecords.push_back(rec);
    sortAndKeepTop10(rotateModeRecords);
}

void RankListWidget::addMultiplayerRecord(int score, int duration, const QString& opponent, bool isWin) {
    RankRecord rec(score, duration, QDateTime::currentDateTime(), opponent, isWin);
    multiplayerRecords.push_back(rec);
    sortAndKeepTop10(multiplayerRecords);
}

void RankListWidget::refreshDisplay() {
    goldenItems.clear();  // 清空以便重新收集
    updateTable(normalModeTable, normalModeRecords, false);
    updateTable(rotateModeTable, rotateModeRecords, false);
    updateTable(multiplayerTable, multiplayerRecords, true);
}

void RankListWidget::onBackClicked() {
    emit backToMenu();
}

void RankListWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    
    // 绘制背景图片
    if (!bgImage.isNull()) {
        // 缩放图片以填充整个窗口，保持比例
        QPixmap scaled = bgImage.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // 居中裁剪
        int offsetX = (scaled.width() - width()) / 2;
        int offsetY = (scaled.height() - height()) / 2;
        p.drawPixmap(0, 0, scaled, offsetX, offsetY, width(), height());
        
        // 添加半透明遮罩层，确保排行榜内容清晰可见
        p.fillRect(rect(), QColor(0, 0, 0, 120));
    } else {
        // 如果没有背景图片，使用渐变背景
        QLinearGradient grad(rect().topLeft(), rect().bottomRight());
        grad.setColorAt(0.0, QColor(25, 25, 45));
        grad.setColorAt(0.5, QColor(35, 35, 65));
        grad.setColorAt(1.0, QColor(45, 30, 70));
        p.fillRect(rect(), grad);
        
        // 绘制一些装饰星星
        p.setPen(Qt::NoPen);
        std::srand(12345);
        for (int i = 0; i < 50; ++i) {
            int x = std::rand() % width();
            int y = std::rand() % height();
            int sz = 1 + std::rand() % 3;
            int alpha = 50 + std::rand() % 100;
            p.setBrush(QColor(255, 255, 255, alpha));
            p.drawEllipse(QPoint(x, y), sz, sz);
        }
    }
}

void RankListWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    update(); // 重绘背景
}
