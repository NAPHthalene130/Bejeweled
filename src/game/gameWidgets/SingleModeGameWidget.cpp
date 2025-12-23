#include "SingleModeGameWidget.h"
#include "FinalWidget.h"
#include "MenuWidget.h"
#include "../GameWindow.h"
#include "../components/Gemstone.h"
#include "../components/SelectedCircle.h"
#include "../../utils/AudioManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QDialog>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DCore/QTransform>
#include <QRandomGenerator>
#include <QVector3D>
#include <QMouseEvent>
#include <QTextEdit>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QHideEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <QApplication>
#include <cmath>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class GameBackDialog : public QDialog {
public:
    explicit GameBackDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        setAttribute(Qt::WA_TranslucentBackground);
        setModal(true);
        setFixedSize(420, 240);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(26, 24, 26, 22);
        layout->setSpacing(14);

        auto* titleLabel = new QLabel("提示", this);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleFont.setFamily("Microsoft YaHei");
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignHCenter);
        titleLabel->setStyleSheet("color: rgba(255,255,255,235); background: transparent;");

        auto* contentLabel = new QLabel("是否返回至主菜单", this);
        QFont contentFont = contentLabel->font();
        contentFont.setPointSize(12);
        contentFont.setFamily("Microsoft YaHei");
        contentLabel->setFont(contentFont);
        contentLabel->setAlignment(Qt::AlignHCenter);
        contentLabel->setWordWrap(true);
        contentLabel->setStyleSheet("color: rgba(255,255,255,210); background: transparent;");

        auto* btnRow = new QHBoxLayout();
        btnRow->setSpacing(14);

        auto* yesBtn = new QPushButton("是", this);
        auto* noBtn = new QPushButton("否", this);

        QSize btnSize(150, 44);
        yesBtn->setFixedSize(btnSize);
        noBtn->setFixedSize(btnSize);
        yesBtn->setCursor(Qt::PointingHandCursor);
        noBtn->setCursor(Qt::PointingHandCursor);

        yesBtn->setStyleSheet(R"(
            QPushButton {
                color: white;
                border-radius: 12px;
                border: 1px solid rgba(255,255,255,60);
                background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(80,180,255,220), stop:1 rgba(80,120,255,220));
                font-family: 'Microsoft YaHei';
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(100,200,255,240), stop:1 rgba(95,140,255,240));
            }
            QPushButton:pressed {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(70,160,220,240), stop:1 rgba(70,110,220,240));
            }
        )");

        noBtn->setStyleSheet(R"(
            QPushButton {
                color: rgba(255,255,255,235);
                border-radius: 12px;
                border: 1px solid rgba(255,255,255,60);
                background: rgba(255,255,255,22);
                font-family: 'Microsoft YaHei';
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:hover {
                background: rgba(255,255,255,30);
            }
            QPushButton:pressed {
                background: rgba(255,255,255,18);
            }
        )");

        connect(yesBtn, &QPushButton::clicked, this, &QDialog::accept);
        connect(noBtn, &QPushButton::clicked, this, &QDialog::reject);

        btnRow->addStretch(1);
        btnRow->addWidget(noBtn);
        btnRow->addWidget(yesBtn);
        btnRow->addStretch(1);

        layout->addWidget(titleLabel);
        layout->addWidget(contentLabel);
        layout->addStretch(1);
        layout->addLayout(btnRow);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRect r = rect().adjusted(8, 8, -8, -8);
        QPainterPath path;
        path.addRoundedRect(r, 18, 18);

        QLinearGradient bg(r.topLeft(), r.bottomRight());
        bg.setColorAt(0, QColor(25, 30, 45, 235));
        bg.setColorAt(1, QColor(15, 18, 30, 235));
        painter.fillPath(path, bg);

        QPen pen(QColor(255, 255, 255, 45));
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawPath(path);
    }
};

SingleModeGameWidget::SingleModeGameWidget(QWidget* parent, GameWindow* gameWindow) 
    : QWidget(parent), gameWindow(gameWindow), canOpe(true), nowTimeHave(0), mode(1),
      firstSelectedGemstone(nullptr), secondSelectedGemstone(nullptr), selectedNum(0) {
    
    // 初始化定时器
    timer = new QTimer(this);
    timer->setInterval(16);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (isFinishing) return;
        gameTimeKeeper.tick();
        nowTimeHave = gameTimeKeeper.totalSeconds();
        updateTimeBoard();
    });
    
    // 设置主背景颜色
    setStyleSheet("background-color: rgb(40, 40, 45);");

    // 初始化3D窗口
    game3dWindow = new Qt3DExtras::Qt3DWindow();
    game3dWindow->defaultFrameGraph()->setClearColor(QColor(40, 40, 45)); // 深灰色背景
    
    // 不重复注册 InputAspect，Qt3DWindow 已默认注册

    // 设置3D场景
    setup3DScene();
    
    // 创建3D窗口容器
    container3d = QWidget::createWindowContainer(game3dWindow);
    container3d->setFixedSize(960, 960);
    container3d->setFocusPolicy(Qt::StrongFocus);
    container3d->setMouseTracking(true); // 启用鼠标追踪
    container3d->setAttribute(Qt::WA_Hover, true); // 启用hover事件
    
    // 布局 - 左侧居中
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(50, 0, 50, 0); // 添加一些边距
    
    // 将容器对齐到左侧，垂直居中
    mainLayout->addWidget(container3d, 0, Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addStretch(1);
    
    rightPanel = new QWidget(this);
    rightPanel->setFixedWidth(420);
    rightPanel->setStyleSheet(R"(
        QWidget {
            background-color: rgba(12, 14, 24, 120);
            border: 1px solid rgba(255, 255, 255, 35);
            border-radius: 18px;
        }
    )");
    auto* panelShadow = new QGraphicsDropShadowEffect(rightPanel);
    panelShadow->setBlurRadius(26);
    panelShadow->setOffset(0, 10);
    panelShadow->setColor(QColor(0, 0, 0, 140));
    rightPanel->setGraphicsEffect(panelShadow);

    auto* panelLayout = new QVBoxLayout(rightPanel);
    panelLayout->setContentsMargins(18, 18, 18, 18);
    panelLayout->setSpacing(16);

    auto* infoCard = new QWidget(rightPanel);
    infoCard->setStyleSheet(R"(
        QWidget {
            background-color: rgba(255, 255, 255, 18);
            border: 1px solid rgba(255, 255, 255, 40);
            border-radius: 16px;
        }
    )");
    auto* infoShadow = new QGraphicsDropShadowEffect(infoCard);
    infoShadow->setBlurRadius(18);
    infoShadow->setOffset(0, 7);
    infoShadow->setColor(QColor(0, 0, 0, 120));
    infoCard->setGraphicsEffect(infoShadow);

    auto* infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(18, 16, 18, 16);
    infoLayout->setSpacing(10);

    scoreBoardLabel = new QLabel(infoCard);
    QFont scoreFont = scoreBoardLabel->font();
    scoreFont.setFamily("Microsoft YaHei");
    scoreFont.setPointSize(16);
    scoreFont.setBold(true);
    scoreBoardLabel->setFont(scoreFont);
    scoreBoardLabel->setStyleSheet("color: rgba(255,255,255,235); background: transparent;");

    timeBoardLabel = new QLabel(infoCard);
    QFont timeFont = timeBoardLabel->font();
    timeFont.setFamily("Microsoft YaHei");
    timeFont.setPointSize(12);
    timeFont.setBold(true);
    timeBoardLabel->setFont(timeFont);
    timeBoardLabel->setStyleSheet("color: rgba(255,255,255,200); background: transparent;");

    infoLayout->addWidget(scoreBoardLabel);
    infoLayout->addWidget(timeBoardLabel);
    panelLayout->addWidget(infoCard, 0, Qt::AlignTop);

    panelLayout->addStretch(1);

    backToMenuButton = new QPushButton("返回菜单", rightPanel);
    backToMenuButton->setFixedSize(180, 54);
    backToMenuButton->setCursor(Qt::PointingHandCursor);
    backToMenuButton->setStyleSheet(R"(
        QPushButton {
            color: white;
            border-radius: 14px;
            border: 1px solid rgba(255,255,255,55);
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(255,140,120,220), stop:1 rgba(255,95,120,220));
            font-family: 'Microsoft YaHei';
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(255,160,130,235), stop:1 rgba(255,115,140,235));
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 rgba(220,110,100,235), stop:1 rgba(220,75,95,235));
        }
    )");

    auto* backShadow = new QGraphicsDropShadowEffect(backToMenuButton);
    backShadow->setBlurRadius(18);
    backShadow->setOffset(0, 8);
    backShadow->setColor(QColor(0, 0, 0, 120));
    backToMenuButton->setGraphicsEffect(backShadow);

    connect(backToMenuButton, &QPushButton::clicked, this, [this]() {
        GameBackDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            if (timer && timer->isActive()) timer->stop();
            if (inactivityTimer) inactivityTimer->stop();
            if (this->gameWindow && this->gameWindow->getMenuWidget()) {
                this->gameWindow->switchWidget(this->gameWindow->getMenuWidget());
            }
        }
    });

    panelLayout->addWidget(backToMenuButton, 0, Qt::AlignRight | Qt::AlignBottom);

    focusInfoLabel = new QLabel(rightPanel);
    focusInfoLabel->setVisible(false);
    debugText = new QTextEdit(rightPanel);
    debugText->setVisible(false);
    debugText->setReadOnly(true);
    debugTimer = new QTimer(this);

    setLayout(mainLayout);
    mainLayout->addWidget(rightPanel, 0, Qt::AlignRight | Qt::AlignVCenter);
    container3d->installEventFilter(this);
    game3dWindow->installEventFilter(this); // 关键：在3D窗口上安装事件过滤器

    // 初始化无操作计时器
    inactivityTimer = new QTimer(this);
    inactivityTimer->setInterval(inactivityTimeout);
    inactivityTimer->setSingleShot(true);
    
    // 超时后高亮可消除的宝石
    connect(inactivityTimer, &QTimer::timeout, this, &SingleModeGameWidget::highlightMatches);
    
    // 任何用户操作后重置计时器
    connect(this, &SingleModeGameWidget::userActionOccurred, [this]() {
        resetInactivityTimer();
    });

    inactivityTimer->stop();

    updateScoreBoard();
    updateTimeBoard();
    appendDebug("SingleModeGameWidget initialized - EventFilter installed on both container and 3D window");
}

void SingleModeGameWidget::GameTimeKeeper::reset() {
    seconds = 0;
}

void SingleModeGameWidget::GameTimeKeeper::tick() {
    ++seconds;
}

int SingleModeGameWidget::GameTimeKeeper::totalSeconds() const {
    return seconds;
}

QString SingleModeGameWidget::GameTimeKeeper::displayText() const {
    int m = seconds / 60;
    int s = seconds % 60;
    return QString("游戏进行时间：%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

void SingleModeGameWidget::updateScoreBoard() {
    if (!scoreBoardLabel) return;
    scoreBoardLabel->setText(QString("当前分数：%1").arg(gameScore));
}

void SingleModeGameWidget::updateTimeBoard() {
    if (!timeBoardLabel) return;
    timeBoardLabel->setText(gameTimeKeeper.displayText());
}

void SingleModeGameWidget::triggerFinishIfNeeded() {
    if (isFinishing) return;
    if (gameScore < targetScore) return;
    finishToFinalWidget();
}

void SingleModeGameWidget::finishToFinalWidget() {
    if (isFinishing) return;
    isFinishing = true;
    canOpe = false;

    if (timer && timer->isActive()) timer->stop();
    if (inactivityTimer) inactivityTimer->stop();
    clearHighlights();
    if (selectionRing1) selectionRing1->setVisible(false);
    if (selectionRing2) selectionRing2->setVisible(false);

    int total = gameTimeKeeper.totalSeconds();
    int m = total / 60;
    int s = total % 60;
    QString timeText = QString("%1:%2")
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));

    QString gradeText = QString("本局得分：%1\n用时：%2\n评价：Excellent!")
        .arg(gameScore)
        .arg(timeText);

    QTimer::singleShot(650, this, [this, gradeText]() {
        if (!gameWindow) return;
        auto* finalWidget = gameWindow->getFinalWidget();
        if (!finalWidget) return;
        finalWidget->setGradeContent(gradeText.toStdString());
        gameWindow->switchWidget(finalWidget);
    });
}

// 查找所有需要消除的宝石（三连或更多）
std::vector<std::pair<int, int>> SingleModeGameWidget::findMatches() {
    std::vector<std::pair<int, int>> matches;
    std::vector<std::vector<bool>> marked(8, std::vector<bool>(8, false));

    // 检查水平方向
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 6; ++j) {  // 最多检查到j=5，这样j+2不会越界
            Gemstone* gem1 = gemstoneContainer[i][j];
            Gemstone* gem2 = gemstoneContainer[i][j+1];
            Gemstone* gem3 = gemstoneContainer[i][j+2];

            if (gem1 && gem2 && gem3 &&
                gem1->getType() == gem2->getType() &&
                gem2->getType() == gem3->getType()) {

                // 标记这三个位置
                marked[i][j] = true;
                marked[i][j+1] = true;
                marked[i][j+2] = true;

                // 继续检查是否有更多连续的
                int k = j + 3;
                while (k < 8 && gemstoneContainer[i][k] &&
                       gemstoneContainer[i][k]->getType() == gem1->getType()) {
                    marked[i][k] = true;
                    k++;
                }
            }
        }
    }

    // 检查垂直方向
    for (int j = 0; j < 8; ++j) {
        for (int i = 0; i < 6; ++i) {  // 最多检查到i=5
            Gemstone* gem1 = gemstoneContainer[i][j];
            Gemstone* gem2 = gemstoneContainer[i+1][j];
            Gemstone* gem3 = gemstoneContainer[i+2][j];

            if (gem1 && gem2 && gem3 &&
                gem1->getType() == gem2->getType() &&
                gem2->getType() == gem3->getType()) {

                // 标记这三个位置
                marked[i][j] = true;
                marked[i+1][j] = true;
                marked[i+2][j] = true;

                // 继续检查是否有更多连续的
                int k = i + 3;
                while (k < 8 && gemstoneContainer[k][j] &&
                       gemstoneContainer[k][j]->getType() == gem1->getType()) {
                    marked[k][j] = true;
                    k++;
                }
            }
        }
    }

    // 收集所有被标记的位置
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (marked[i][j]) {
                matches.push_back({i, j});
            }
        }
    }

    return matches;
}

// 移除匹配的宝石
void SingleModeGameWidget::removeMatches(const std::vector<std::pair<int, int>>& matches) {
    if (matches.empty()) {
        appendDebug("No matches to remove");
        return;
    }

    appendDebug(QString("Removing %1 gemstones").arg(matches.size()));

    int removedCount = 0;
    for (const auto& pos : matches) {
        int row = pos.first;
        int col = pos.second;
        Gemstone* gem = gemstoneContainer[row][col];

        if (gem) {
            removedCount += 1;
            // 播放消除动画
            eliminateAnime(gem);
            // 从容器中移除
            gemstoneContainer[row][col] = nullptr;
        }
    }

    if (removedCount > 0) {
        gameScore += removedCount * 10;
        updateScoreBoard();
        triggerFinishIfNeeded();
    }
}

int comboCount = 0;

void SingleModeGameWidget::eliminate() {
    if (isFinishing) return;
    // 查找所有匹配
    std::vector<std::pair<int, int>> matches = findMatches();
    if (!matches.empty()) {
        comboCount++; // 增加连续消除计数
        appendDebug(QString("Found %1 matches to eliminate").arg(matches.size()));
        AudioManager::instance().playEliminateSound(comboCount);


        // 禁止操作
        canOpe = false;
        
        // 移除匹配的宝石
        removeMatches(matches);

        if (isFinishing) return;

        // 等待消除动画完成后执行下落（500ms）
        QTimer::singleShot(600, this, [this]() {
            drop();
        });
    } else {
        comboCount = 0;
        // 没有匹配了，恢复操作
        canOpe = true;
        resetInactivityTimer();
        appendDebug("No matches found, game can continue");
    }
}

void SingleModeGameWidget::drop() {
    if (isFinishing) return;
    appendDebug("Starting drop animation");

    QParallelAnimationGroup* dropAnimGroup = new QParallelAnimationGroup();
    bool hasDrops = false;

    for (int col = 0; col < 8; ++col) {
        int writePos = 7; // 从底部开始写入
        for (int row = 7; row >= 0; --row) {
            if (gemstoneContainer[row][col] != nullptr) {
                if (row < writePos) {
                    Gemstone* gem = gemstoneContainer[row][col];
                    gemstoneContainer[writePos][col] = gem;
                    gemstoneContainer[row][col] = nullptr;

                    QVector3D targetPos = getPosition(writePos, col);
                    QPropertyAnimation* dropAnim = new QPropertyAnimation(gem->transform(), "translation");
                    dropAnim->setDuration(500);
                    dropAnim->setStartValue(gem->transform()->translation());
                    dropAnim->setEndValue(targetPos);
                    dropAnimGroup->addAnimation(dropAnim);

                    hasDrops = true;
                }
                writePos--;
            }
        }
    }

    if (hasDrops) {
        appendDebug("Drop animation started");
        connect(dropAnimGroup, &QParallelAnimationGroup::finished, this, [this]() {
            if (isFinishing) return;
            appendDebug("Drop animation finished, filling new gemstones");
            resetGemstoneTable();
            resetInactivityTimer();
        });
        dropAnimGroup->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        appendDebug("No drops needed, filling new gemstones");
        resetGemstoneTable();
        resetInactivityTimer();
    }
}

void SingleModeGameWidget::resetGemstoneTable() {
    if (isFinishing) return;
    appendDebug("Filling empty positions with new gemstones");

    QParallelAnimationGroup* fillAnimGroup = new QParallelAnimationGroup();
    bool hasFills = false;

    // 遍历所有位置，找到空位并填充新宝石
    for (int col = 0; col < 8; ++col) {
        for (int row = 0; row < 8; ++row) {
            if (gemstoneContainer[row][col] == nullptr) {
                // 创建新宝石，避免立即形成三连
                int type = QRandomGenerator::global()->bounded(difficulty);

                // 检查左边两个
                if (col >= 2 && gemstoneContainer[row][col-1] && gemstoneContainer[row][col-2]) {
                    int type1 = gemstoneContainer[row][col-1]->getType();
                    int type2 = gemstoneContainer[row][col-2]->getType();
                    if (type1 == type2 && type == type1) {
                        type = (type + 1) % difficulty;
                    }
                }

                // 检查上边两个
                if (row >= 2 && gemstoneContainer[row-1][col] && gemstoneContainer[row-2][col]) {
                    int type1 = gemstoneContainer[row-1][col]->getType();
                    int type2 = gemstoneContainer[row-2][col]->getType();
                    if (type1 == type2 && type == type1) {
                        type = (type + 1) % difficulty;
                    }
                }

                Gemstone* gem = new Gemstone(type, "default", rootEntity);

                // 从上方一个位置开始（制造下落效果）
                QVector3D startPos = getPosition(row - 3, col); // 从更高的位置开始
                QVector3D targetPos = getPosition(row, col);

                gem->transform()->setTranslation(startPos);

                // 连接点击信号
                connect(gem, &Gemstone::clicked, this, &SingleModeGameWidget::handleGemstoneClicked);
                connect(gem, &Gemstone::pickEvent, this, [this](const QString& info) {
                    appendDebug(QString("Gemstone %1").arg(info));
                });

                gemstoneContainer[row][col] = gem;

                // 创建下落动画
                QPropertyAnimation* fillAnim = new QPropertyAnimation(gem->transform(), "translation");
                fillAnim->setDuration(500);
                fillAnim->setStartValue(startPos);
                fillAnim->setEndValue(targetPos);
                fillAnimGroup->addAnimation(fillAnim);

                hasFills = true;
            }
        }
    }

    if (hasFills) {
        appendDebug("Fill animation started");
        connect(fillAnimGroup, &QParallelAnimationGroup::finished, this, [this]() {
            if (isFinishing) return;
            appendDebug("Fill animation finished, checking for new matches");
            // 填充完成后，递归检查是否有新的匹配
            eliminate();
        });
        fillAnimGroup->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        appendDebug("No fills needed, checking for new matches");
        // 没有填充，直接检查匹配
        eliminate();
    }
}

void SingleModeGameWidget::eliminateAnime(Gemstone* gemstone) {
    if (!gemstone) return;
    
    QPropertyAnimation* animation = new QPropertyAnimation(gemstone->transform(), "scale");
    animation->setDuration(500); // 持续缩小直到不见
    animation->setStartValue(gemstone->transform()->scale());
    animation->setEndValue(0.0f);
    
    connect(animation, &QPropertyAnimation::finished, [gemstone]() {
        gemstone->setParent((Qt3DCore::QNode*)nullptr);
        delete gemstone;
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void SingleModeGameWidget::switchGemstoneAnime(Gemstone* gemstone1, Gemstone* gemstone2) {
    if (!gemstone1 || !gemstone2) return;
    
    QVector3D pos1 = gemstone1->transform()->translation();
    QVector3D pos2 = gemstone2->transform()->translation();
    
    QParallelAnimationGroup* group = new QParallelAnimationGroup();
    
    QPropertyAnimation* anim1 = new QPropertyAnimation(gemstone1->transform(), "translation");
    anim1->setDuration(500); // 0.5s
    anim1->setStartValue(pos1);
    anim1->setEndValue(pos2);
    
    QPropertyAnimation* anim2 = new QPropertyAnimation(gemstone2->transform(), "translation");
    anim2->setDuration(500); // 0.5s
    anim2->setStartValue(pos2);
    anim2->setEndValue(pos1);
    
    group->addAnimation(anim1);
    group->addAnimation(anim2);
    
    connect(group, &QParallelAnimationGroup::finished, [this]() {
        syncGemstonePositions();
    });
    
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void SingleModeGameWidget::handleGemstoneClicked(Gemstone* gem) {
    if (!gem) {
        appendDebug("handleGemstoneClicked: gem is null!");
        return;
    }
    if (!canOpe) return;
    emit userActionOccurred(); // 发送用户操作信号

    appendDebug(QString("Gemstone clicked! Type=%1 Mode=%2 CanOpe=%3 SelectedNum=%4")
        .arg(gem->getType()).arg(mode).arg(canOpe).arg(selectedNum));

    if (mode != 1 || !canOpe) {
        appendDebug(QString("Click ignored: mode=%1 canOpe=%2").arg(mode).arg(canOpe));
        return;
    }

    // 避免重复选择同一个
    if (gem == firstSelectedGemstone || gem == secondSelectedGemstone) {
        appendDebug("Same gemstone clicked, ignoring");
        return;
    }

    if (selectedNum == 0) {
        selectedNum = 1;
        firstSelectedGemstone = gem;
        // 显示第一个选择框
        selectionRing1->setPosition(gem->transform()->translation());
        selectionRing1->setVisible(true);
        appendDebug(QString("First gemstone selected at (%.2f, %.2f)")
            .arg(gem->transform()->translation().x())
            .arg(gem->transform()->translation().y()));
    } else if (selectedNum == 1) {
        selectedNum = 2;
        secondSelectedGemstone = gem;
        // 显示第二个选择框
        selectionRing2->setPosition(gem->transform()->translation());
        selectionRing2->setVisible(true);
        appendDebug(QString("Second gemstone selected at (%.2f, %.2f)")
            .arg(gem->transform()->translation().x())
            .arg(gem->transform()->translation().y()));

        // 自动触发交换逻辑
        int row1 = -1, col1 = -1, row2 = -1, col2 = -1;
        if (findGemstonePosition(firstSelectedGemstone, row1, col1) &&
            findGemstonePosition(secondSelectedGemstone, row2, col2)) {

            appendDebug(QString("Found positions: (%1,%2) and (%3,%4)")
                .arg(row1).arg(col1).arg(row2).arg(col2));

            if (areAdjacent(row1, col1, row2, col2)) {
                appendDebug("Gems are adjacent, performing swap!");
                performSwap(firstSelectedGemstone, secondSelectedGemstone, row1, col1, row2, col2);
            } else {
                appendDebug("Gems are NOT adjacent, clearing selection");
                // 不相邻，清除选择
                firstSelectedGemstone = nullptr;
                secondSelectedGemstone = nullptr;
                selectedNum = 0;
                selectionRing1->setVisible(false);
                selectionRing2->setVisible(false);
            }
        } else {
            appendDebug("ERROR: Could not find gemstone positions!");
        }
    }
}

void SingleModeGameWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        if (mode == 1) {
            // 取消选择
            if (firstSelectedGemstone) {
                firstSelectedGemstone = nullptr;
                selectionRing1->setVisible(false);
            }
            if (secondSelectedGemstone) {
                secondSelectedGemstone = nullptr;
                selectionRing2->setVisible(false);
            }
            selectedNum = 0;
            this->setWindowTitle("Selection Cleared");
        }
    }
    QWidget::mousePressEvent(event);
}

void SingleModeGameWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (container3d) {
        container3d->setFocus(Qt::OtherFocusReason);
        container3d->raise();
    }
    refreshDebugStatus();
    appendDebug("showEvent");
    updateScoreBoard();
    updateTimeBoard();
    if (!isFinishing && timer && !timer->isActive()) {
        timer->start();
    }
    resetInactivityTimer();
}

void SingleModeGameWidget::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (timer && timer->isActive()) {
        timer->stop();
    }
    if (inactivityTimer) {
        inactivityTimer->stop();
    }
    clearHighlights();
}

bool SingleModeGameWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == container3d) {
        if (event->type() == QEvent::FocusIn) {
            refreshDebugStatus();
            appendDebug("container3d FocusIn");
        } else if (event->type() == QEvent::FocusOut) {
            refreshDebugStatus();
            appendDebug("container3d FocusOut");
        }
    } else if (obj == game3dWindow) {
        // 处理来自3D窗口的事件
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            appendDebug(QString("game3dWindow MouseButtonPress at (%1, %2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));

            // 手动处理点击 - 转换屏幕坐标到世界坐标
            handleManualClick(mouseEvent->pos());
            refreshDebugStatus();
            return false; // 不消费事件，让Qt3D也能处理
        } else if (event->type() == QEvent::MouseMove) {
            // 追踪鼠标移动以确认事件被接收
            static int moveCount = 0;
            if (++moveCount % 50 == 0) { // 每50次移动输出一次
                appendDebug("Mouse moving over 3D window");
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

SingleModeGameWidget::~SingleModeGameWidget() {
    clearHighlights();
    delete inactivityTimer;

    if (rootEntity) {
        delete rootEntity;
    }
    if (game3dWindow) {
        delete game3dWindow;
    }
    if (debugTimer && debugTimer->isActive()) {
        debugTimer->stop();
    }
}

#include <Qt3DRender/QPickingSettings>
#include <Qt3DRender/QRenderSettings>

void SingleModeGameWidget::setup3DScene() {
    // 根实体
    rootEntity = new Qt3DCore::QEntity();

    // 设置根实体
    game3dWindow->setRootEntity(rootEntity);

    // 尝试配置picking（如果renderSettings可用）
    Qt3DRender::QRenderSettings *renderSettings = game3dWindow->renderSettings();
    if (renderSettings) {
        Qt3DRender::QPickingSettings *pickingSettings = renderSettings->pickingSettings();
        if (pickingSettings) {
            pickingSettings->setPickMethod(Qt3DRender::QPickingSettings::TrianglePicking);
            pickingSettings->setPickResultMode(Qt3DRender::QPickingSettings::NearestPick);
            pickingSettings->setFaceOrientationPickingMode(Qt3DRender::QPickingSettings::FrontAndBackFace);
        }
    }

    // 相机
    cameraEntity = game3dWindow->camera();
    cameraEntity->lens()->setPerspectiveProjection(45.0f, 1.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0.0f, 0.0f, 20.0f));
    cameraEntity->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    // 灯光
    lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(1.0f);
    lightEntity->addComponent(light);

    Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(0.0f, 0.0f, 20.0f));
    lightEntity->addComponent(lightTransform);

    // 初始化选择框
    selectionRing1 = new SelectedCircle(rootEntity);
    selectionRing2 = new SelectedCircle(rootEntity);

    qDebug() << "[SingleModeGameWidget] 3D Scene setup complete - InputSettings and PickingSettings configured";
}

QVector3D SingleModeGameWidget::getPosition(int row, int col) const {
    float startX = -3.5f * 1.5f; // 居中网格
    float startY = 3.5f * 1.5f;
    float spacing = 1.5f;
    
    // 设置位置：[0][0] 是左上角
    // i 是行 (Y), j 是列 (X)
    // 在3D中：X向右增加，Y向上增加。
    // 所以列 j 映射到 X，行 i 映射到 -Y（向下）
    
    float x = startX + col * spacing;
    float y = startY - row * spacing;
    
    return QVector3D(x, y, 0.0f);
}

void SingleModeGameWidget::syncGemstonePositions() {
    for (int i = 0; i < gemstoneContainer.size(); ++i) {
        for (int j = 0; j < gemstoneContainer[i].size(); ++j) {
            Gemstone* gem = gemstoneContainer[i][j];
            if (gem) {
                // 停止该宝石上可能正在运行的任何位置动画
                // 注意：这里我们假设直接设置位置会覆盖正在进行的动画，
                // 或者动画已经结束。如果动画还在运行，直接设置可能会导致冲突，
                // 但Qt3D的transform通常是即时的。
                // 更好的做法是确保没有动画在控制它。
                
                gem->transform()->setTranslation(getPosition(i, j));
            }
        }
    }
}

Qt3DExtras::Qt3DWindow* SingleModeGameWidget::getGame3dWindow() const {
    return game3dWindow;
}

std::vector<std::vector<Gemstone*>> SingleModeGameWidget::getGemstoneContainer() const {
    return gemstoneContainer;
}

void SingleModeGameWidget::setGemstoneContainer(const std::vector<std::vector<Gemstone*>>& container) {
    this->gemstoneContainer = container;
}

std::string SingleModeGameWidget::getStyle() const {
    return style;
}

void SingleModeGameWidget::setStyle(const std::string& style) {
    this->style = style;
}

bool SingleModeGameWidget::getCanOpe() const {
    return canOpe;
}

void SingleModeGameWidget::setCanOpe(bool canOpe) {
    this->canOpe = canOpe;
}

QTimer* SingleModeGameWidget::getTimer() const {
    return timer;
}

void SingleModeGameWidget::setTimer(QTimer* timer) {
    this->timer = timer;
}

int SingleModeGameWidget::getNowTimeHave() const {
    return nowTimeHave;
}

void SingleModeGameWidget::setNowTimeHave(int time) {
    this->nowTimeHave = time;
}

int SingleModeGameWidget::getMode() const {
    return mode;
}

void SingleModeGameWidget::setMode(int mode) {
    this->mode = mode;
}

void SingleModeGameWidget::reset(int mode) {
    this->mode = mode;
    this->canOpe = true;
    this->isFinishing = false;
    this->gameScore = 0;
    this->targetScore = 10000;
    this->gameTimeKeeper.reset();
    this->nowTimeHave = 0;
    updateScoreBoard();
    updateTimeBoard();
    appendDebug(QString("reset mode=%1").arg(mode));
    
    // 清除现有的宝石（如果有）
    for (auto& row : gemstoneContainer) {
        for (auto* gem : row) {
            if (gem) {
                gem->setParent((Qt3DCore::QNode*)nullptr); // 从场景中分离
                delete gem;
            }
        }
    }
    gemstoneContainer.clear();
    
    // 重建8x8网格
    gemstoneContainer.resize(8);

    for (int i = 0; i < 8; ++i) {
        gemstoneContainer[i].resize(8);
        for (int j = 0; j < 8; ++j) {
            int type = QRandomGenerator::global()->bounded(difficulty);

            // 避免在初始化时创建三连
            // 检查左边两个
            if (j >= 2 && gemstoneContainer[i][j-1] && gemstoneContainer[i][j-2]) {
                int type1 = gemstoneContainer[i][j-1]->getType();
                int type2 = gemstoneContainer[i][j-2]->getType();
                if (type1 == type2 && type == type1) {
                    // 会形成三连，换一个类型
                    type = (type + 1) % difficulty;
                }
            }

            // 检查上边两个
            if (i >= 2 && gemstoneContainer[i-1][j] && gemstoneContainer[i-2][j]) {
                int type1 = gemstoneContainer[i-1][j]->getType();
                int type2 = gemstoneContainer[i-2][j]->getType();
                if (type1 == type2 && type == type1) {
                    // 会形成三连，换一个类型
                    type = (type + 1) % difficulty;
                }
            }

            Gemstone* gem = new Gemstone(type, "default", rootEntity);

            gem->transform()->setTranslation(getPosition(i, j));

            // 连接点击信号
            connect(gem, &Gemstone::clicked, this, &SingleModeGameWidget::handleGemstoneClicked);
            connect(gem, &Gemstone::pickEvent, this, [this](const QString& info) { appendDebug(QString("Gemstone %1").arg(info)); });

            gemstoneContainer[i][j] = gem;
        }
    }
    appendDebug("created 8x8 gemstones with no initial matches");
    
    // 重置选择状态
    selectedNum = 0;
    firstSelectedGemstone = nullptr;
    secondSelectedGemstone = nullptr;

    // 重置定时器
    if (timer->isActive()) {
        timer->stop();
    }
}

void SingleModeGameWidget::appendDebug(const QString& text) {
    if (!debugText) return; // 防止在初始化之前调用
    debugText->append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(text));
}

void SingleModeGameWidget::refreshDebugStatus() {
    if (!focusInfoLabel) return;
    bool hasFocusContainer = container3d ? container3d->hasFocus() : false;
    QWidget* active = QApplication::activeWindow();
    QString activeTitle = active ? active->windowTitle() : QString("null");
    focusInfoLabel->setText(QString("ContainerFocus=%1 | ActiveWindow=%2").arg(hasFocusContainer ? "true" : "false").arg(activeTitle));
}

// 找到宝石在容器中的位置
bool SingleModeGameWidget::findGemstonePosition(Gemstone* gem, int& row, int& col) const {
    if (!gem) return false;

    for (int i = 0; i < gemstoneContainer.size(); ++i) {
        for (int j = 0; j < gemstoneContainer[i].size(); ++j) {
            if (gemstoneContainer[i][j] == gem) {
                row = i;
                col = j;
                return true;
            }
        }
    }
    return false;
}

// 检查两个位置是否相邻（上下左右）
bool SingleModeGameWidget::areAdjacent(int row1, int col1, int row2, int col2) const {
    int rowDiff = std::abs(row1 - row2);
    int colDiff = std::abs(col1 - col2);

    // 相邻的条件：要么行相同且列相差1，要么列相同且行相差1
    return (rowDiff == 1 && colDiff == 0) || (rowDiff == 0 && colDiff == 1);
}

// 执行交换
void SingleModeGameWidget::performSwap(Gemstone* gem1, Gemstone* gem2, int row1, int col1, int row2, int col2) {
    if (!gem1 || !gem2) return;

    // 先在逻辑容器中交换
    gemstoneContainer[row1][col1] = gem2;
    gemstoneContainer[row2][col2] = gem1;

    // 播放交换动画
    QVector3D pos1 = gem1->transform()->translation();
    QVector3D pos2 = gem2->transform()->translation();

    QParallelAnimationGroup* group = new QParallelAnimationGroup();

    QPropertyAnimation* anim1 = new QPropertyAnimation(gem1->transform(), "translation");
    anim1->setDuration(500);
    anim1->setStartValue(pos1);
    anim1->setEndValue(pos2);

    QPropertyAnimation* anim2 = new QPropertyAnimation(gem2->transform(), "translation");
    anim2->setDuration(500);
    anim2->setStartValue(pos2);
    anim2->setEndValue(pos1);

    group->addAnimation(anim1);
    group->addAnimation(anim2);

    connect(group, &QParallelAnimationGroup::finished, this, [this, gem1, gem2, row1, col1, row2, col2]() {
        appendDebug("Swap animation finished, checking for matches");

        // 检查是否有匹配
        std::vector<std::pair<int, int>> matches = findMatches();

        if (!matches.empty()) {
            // 有匹配，触发消除
            appendDebug(QString("Found matches after swap, starting elimination"));
            eliminate();
        } else {
            // 没有匹配，交换回来
            appendDebug("No matches found, swapping back");

            // 在逻辑容器中交换回来
            gemstoneContainer[row1][col1] = gem1;
            gemstoneContainer[row2][col2] = gem2;

            // 播放交换回来的动画
            QVector3D pos1 = gem1->transform()->translation();
            QVector3D pos2 = gem2->transform()->translation();

            QParallelAnimationGroup* swapBackGroup = new QParallelAnimationGroup();

            QPropertyAnimation* backAnim1 = new QPropertyAnimation(gem1->transform(), "translation");
            backAnim1->setDuration(500);
            backAnim1->setStartValue(pos1);
            backAnim1->setEndValue(pos2);

            QPropertyAnimation* backAnim2 = new QPropertyAnimation(gem2->transform(), "translation");
            backAnim2->setDuration(500);
            backAnim2->setStartValue(pos2);
            backAnim2->setEndValue(pos1);

            swapBackGroup->addAnimation(backAnim1);
            swapBackGroup->addAnimation(backAnim2);

            connect(swapBackGroup, &QParallelAnimationGroup::finished, this, [this]() {
                canOpe = true; // 恢复操作
            });

            swapBackGroup->start(QAbstractAnimation::DeleteWhenStopped);
        }
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);

    // 清除选择状态
    firstSelectedGemstone = nullptr;
    secondSelectedGemstone = nullptr;
    selectedNum = 0;
    selectionRing1->setVisible(false);
    selectionRing2->setVisible(false);

    appendDebug(QString("Swapped gems at (%1,%2) and (%3,%4)").arg(row1).arg(col1).arg(row2).arg(col2));
}

// 手动处理鼠标点击 - 将屏幕坐标转换为世界坐标并找到最近的宝石
void SingleModeGameWidget::handleManualClick(const QPoint& screenPos) {
    // 容器大小是 960x960
    float screenWidth = 960.0f;
    float screenHeight = 960.0f;

    // 相机参数：FOV=45度，distance=20，aspect=1.0
    // 计算在z=0平面上的可视范围
    float fovRadians = 45.0f * M_PI / 180.0f;  // 转换为弧度
    float cameraDistance = 20.0f;
    float halfHeight = cameraDistance * std::tan(fovRadians / 2.0f);  // z=0平面上的半高度
    float halfWidth = halfHeight;  // aspect = 1.0

    // 将屏幕坐标归一化到 [-1, 1]
    float normalizedX = (screenPos.x() - screenWidth / 2.0f) / (screenWidth / 2.0f);
    float normalizedY = -(screenPos.y() - screenHeight / 2.0f) / (screenHeight / 2.0f);  // Y轴反向

    // 转换到世界坐标（z=0平面）
    float worldX = normalizedX * halfWidth;
    float worldY = normalizedY * halfHeight;

    appendDebug(QString("Click at screen(%1,%2) -> normalized(%3,%4) -> world(%5, %6)")
        .arg(screenPos.x()).arg(screenPos.y())
        .arg(normalizedX, 0, 'f', 2).arg(normalizedY, 0, 'f', 2)
        .arg(worldX, 0, 'f', 2).arg(worldY, 0, 'f', 2));

    // 找到最接近这个位置的宝石
    Gemstone* closestGem = nullptr;
    float minDistance = std::numeric_limits<float>::max();
    int closestRow = -1, closestCol = -1;

    for (int i = 0; i < gemstoneContainer.size(); ++i) {
        for (int j = 0; j < gemstoneContainer[i].size(); ++j) {
            Gemstone* gem = gemstoneContainer[i][j];
            if (gem) {
                QVector3D gemPos = gem->transform()->translation();
                float dx = gemPos.x() - worldX;
                float dy = gemPos.y() - worldY;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < minDistance) {
                    minDistance = distance;
                    closestGem = gem;
                    closestRow = i;
                    closestCol = j;
                }
            }
        }
    }

    // 如果找到了足够近的宝石（距离 < 0.8，稍微放宽一点）
    if (closestGem && minDistance < 0.8f) {
        appendDebug(QString("Found gemstone at (%1,%2), distance=%3")
            .arg(closestRow).arg(closestCol).arg(minDistance, 0, 'f', 2));
        handleGemstoneClicked(closestGem);
    } else {
        appendDebug(QString("No gemstone found near click (min distance=%1)")
            .arg(minDistance, 0, 'f', 2));
    }
}
// 重置无操作计时器————————————————————————————————————————————————————————————————————————————————————————————————————————————
void SingleModeGameWidget::resetInactivityTimer() {
    clearHighlights();
    if (!inactivityTimer) return;
    if (!isVisible()) {
        inactivityTimer->stop();
        return;
    }
    if (gemstoneContainer.size() != 8) {
        inactivityTimer->stop();
        return;
    }
    for (const auto& row : gemstoneContainer) {
        if (row.size() != 8) {
            inactivityTimer->stop();
            return;
        }
        for (auto* gem : row) {
            if (!gem) {
                inactivityTimer->stop();
                return;
            }
        }
    }
    inactivityTimer->start(inactivityTimeout);
}

std::vector<std::pair<int, int>> SingleModeGameWidget::findPossibleMatches() {
    std::vector<std::pair<int, int>> matches;
    if (gemstoneContainer.size() != 8) return matches;
    for (const auto& row : gemstoneContainer) {
        if (row.size() != 8) return matches;
    }
    std::vector<std::vector<bool>> marked(8, std::vector<bool>(8, false));
    const int dx[4] = {0,0,1,-1};
    const int dy[4] = {1,-1,0,0};
    // 检查水平方向
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 7; ++j) {  // 最多检查到j=5，这样j+2不会越界
            Gemstone* gem1 = gemstoneContainer[i][j];
            Gemstone* gem2 = gemstoneContainer[i][j+1];

            if (gem1 && gem2 && gem1->getType() == gem2->getType()) {
                int L = j-1,R=j+2;
                for(int k = 0;k < 4; k++) {
                    int Dx = i+dx[k],Dy1 = L+dy[k],Dy2 = R+dy[k];
                    if(L>0) {
                        if(Dx >= 0 && Dx < 8 && Dy1 >= 0 && Dy1 < 8 && k!=0) {
                            Gemstone* g = gemstoneContainer[Dx][Dy1];
                            if(g && g->getType() == gem1->getType()) {
                                marked[Dx][Dy1] = true;
                            }
                        }
                    }
                    if(R<8) {
                        if(Dx >= 0 && Dx < 8 && Dy2 >= 0 && Dy2 < 8 && k!=1) {
                            Gemstone* g = gemstoneContainer[Dx][Dy2];
                            if(g && g->getType() == gem1->getType()) {
                                marked[Dx][Dy2] = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // 检查垂直方向
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 8; ++j) {  // 最多检查到j=5，这样j+2不会越界
            Gemstone* gem1 = gemstoneContainer[i][j];
            Gemstone* gem2 = gemstoneContainer[i+1][j];

            if (gem1 && gem2 && gem1->getType() == gem2->getType()) {
                int L = i-1,R=i+2;
                for(int k = 0;k < 4; k++) {
                    int Dx1 = L+dx[k],Dx2 = R+dx[k],Dy = j+dy[k];
                    if(L>0) {
                        if(Dx1 >= 0 && Dx1 < 8 && Dy >= 0 && Dy < 8 && k!=2) {
                            Gemstone* g = gemstoneContainer[Dx1][Dy];
                            if(g && g->getType() == gem1->getType()) {
                                marked[Dx1][Dy] = true;
                            }
                        }
                    }
                    if(R<8) {
                        if(Dx2 >= 0 && Dx2 < 8 && Dy >= 0 && Dy < 8 && k!=3) {
                            Gemstone* g = gemstoneContainer[Dx2][Dy];
                            if(g && g->getType() == gem1->getType()) {
                                marked[Dx2][Dy] = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // 收集所有被标记的位置
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (marked[i][j]) {
                matches.push_back({i, j});
            }
        }
    }

    return matches;
}

// 高亮显示所有可消除的宝石
void SingleModeGameWidget::highlightMatches() {
    if (!canOpe) return; // 操作不可用时不高亮
    if (!isVisible()) return;
    if (gemstoneContainer.size() != 8) return;
    for (const auto& row : gemstoneContainer) {
        if (row.size() != 8) return;
    }
    
    clearHighlights(); // 先清除现有高亮
    
    // 找到所有可消除的宝石
    std::vector<std::pair<int, int>> matches = findPossibleMatches();
    if (matches.empty()) {
        appendDebug("No possible matches found,resetting the game");
        reset(1);
        return ;
    }
    
    appendDebug(QString("No activity detected for %1 seconds, highlighting %2 matches")
               .arg(inactivityTimeout/1000).arg(matches.size()));
    
    // 为随机一个可消除的宝石添加高亮环
    int choice = QRandomGenerator::global()->bounded(matches.size()) ,num = 0;
    for (const auto& pos : matches) {
        int row = pos.first;
        int col = pos.second;
        Gemstone* gem = gemstoneContainer[row][col];
        if (gem && num == choice) {
            SelectedCircle* ring = new SelectedCircle(rootEntity);
            ring->setVisible(true);
            ring->setPosition(getPosition(row, col));
            highlightRings.push_back(ring);
            break;
        }
        num++;
    }
}

// 清除所有高亮
void SingleModeGameWidget::clearHighlights() {
    for (SelectedCircle* ring : highlightRings) {
        ring->setVisible(false);
        delete ring;
    }
    highlightRings.clear();
}

void SingleModeGameWidget::setDifficulty(int diff) {
    difficulty = diff;
}

int SingleModeGameWidget::getDifficulty() const {
    return difficulty;
}
