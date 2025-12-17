#include "SingleModeGameWidget.h"
#include "../components/Gemstone.h"
#include "../components/SelectedCircle.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
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
#include <QDateTime>
#include <QApplication>
#include <cmath>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SingleModeGameWidget::SingleModeGameWidget(QWidget* parent, GameWindow* gameWindow) 
    : QWidget(parent), gameWindow(gameWindow), canOpe(true), nowTimeHave(0), mode(1),
      firstSelectedGemstone(nullptr), secondSelectedGemstone(nullptr), selectedNum(0) {
    
    // 初始化定时器
    timer = new QTimer(this);
    
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
    
    // 右侧控制按钮
    QVBoxLayout* rightLayout = new QVBoxLayout();
    
    QPushButton* backButton = new QPushButton("返回", this);
    QPushButton* testEliminateButton = new QPushButton("消除测试", this);
    QPushButton* testSwitchButton = new QPushButton("交换测试", this);
    
    focusInfoLabel = new QLabel(this);
    debugText = new QTextEdit(this);
    debugText->setReadOnly(true);
    debugText->setMinimumHeight(300);
    debugText->setMinimumWidth(400);
    debugTimer = new QTimer(this);
    connect(debugTimer, &QTimer::timeout, this, &SingleModeGameWidget::refreshDebugStatus);
    debugTimer->start(500);

    rightLayout->addWidget(backButton);
    rightLayout->addWidget(testEliminateButton);
    rightLayout->addWidget(testSwitchButton);
    rightLayout->addSpacing(10);
    rightLayout->addWidget(focusInfoLabel);
    rightLayout->addWidget(debugText);
    rightLayout->addStretch(1);
    
    mainLayout->addLayout(rightLayout);
    mainLayout->addStretch(1); // 将内容推向左侧

    connect(testEliminateButton, &QPushButton::clicked, [this]() {
        // 随机找一个存在的宝石进行消除
        if (gemstoneContainer.empty()) return;
        
        int r = QRandomGenerator::global()->bounded((int)gemstoneContainer.size());
        if (gemstoneContainer[r].empty()) return;
        int c = QRandomGenerator::global()->bounded((int)gemstoneContainer[r].size());
        
        Gemstone* gem = gemstoneContainer[r][c];
        if (gem) {
            eliminateAnime(gem);
            gemstoneContainer[r][c] = nullptr; // 从容器中移除引用，防止野指针
        }
    });

    connect(testSwitchButton, &QPushButton::clicked, [this]() {
        // 随机找两个相邻的宝石进行交换
        // 这里简单起见，找(0,0)和(0,1)或者随机位置
        if (gemstoneContainer.size() < 1 || gemstoneContainer[0].size() < 2) return;
        
        Gemstone* g1 = nullptr;
        Gemstone* g2 = nullptr;
        int r = -1, c = -1;
        
        // 尝试找到两个存在的宝石
        for(int i=0; i<10; ++i) {
            r = QRandomGenerator::global()->bounded((int)gemstoneContainer.size());
            c = QRandomGenerator::global()->bounded((int)gemstoneContainer[r].size() - 1);
            
            g1 = gemstoneContainer[r][c];
            g2 = gemstoneContainer[r][c+1];
            
            if(g1 && g2) break;
        }
        
        if (g1 && g2 && r != -1 && c != -1) {
            // 在动画开始前或结束后交换逻辑容器中的位置
            // 这里我们选择先交换逻辑，然后播放动画
            // 这样当动画结束调用 syncGemstonePositions 时，位置是正确的
            std::swap(gemstoneContainer[r][c], gemstoneContainer[r][c+1]);
            
            switchGemstoneAnime(g1, g2);
        }
    });
    
    setLayout(mainLayout);
    container3d->installEventFilter(this);
    game3dWindow->installEventFilter(this); // 关键：在3D窗口上安装事件过滤器

    appendDebug("SingleModeGameWidget initialized - EventFilter installed on both container and 3D window");
}

void SingleModeGameWidget::eliminate() {
    // TODO
}

void SingleModeGameWidget::drop() {
    // TODO
}

void SingleModeGameWidget::resetGemstoneTable() {
    // TODO
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
    this->nowTimeHave = 0;
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
            int type = QRandomGenerator::global()->bounded(8);
            Gemstone* gem = new Gemstone(type, "default", rootEntity);
            
            gem->transform()->setTranslation(getPosition(i, j));
            
            // 连接点击信号
            connect(gem, &Gemstone::clicked, this, &SingleModeGameWidget::handleGemstoneClicked);
            connect(gem, &Gemstone::pickEvent, this, [this](const QString& info) { appendDebug(QString("Gemstone %1").arg(info)); });

            gemstoneContainer[i][j] = gem;
        }
    }
    appendDebug("created 8x8 gemstones");
    
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
    switchGemstoneAnime(gem1, gem2);

    // 清除选择状态（动画结束后会调用 syncGemstonePositions）
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
