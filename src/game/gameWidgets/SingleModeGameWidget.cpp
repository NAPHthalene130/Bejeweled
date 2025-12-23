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
    if (mode != 1 || !canOpe) return;

    // 如果已经选中了两个，不再接受新的选择，等待逻辑处理（虽然题目说超过2不再添加，但通常意味着不处理）
    // 或者如果点击的是已经选中的，可能需要取消选择？题目没说，先假设只处理添加。
    
    // 避免重复选择同一个
    if (gem == firstSelectedGemstone || gem == secondSelectedGemstone) return;

    if (selectedNum == 0) {
        selectedNum = 1;
        firstSelectedGemstone = gem;
        // 显示第一个选择框
        selectionRing1->setPosition(gem->transform()->translation());
        selectionRing1->setVisible(true);
    } else if (selectedNum == 1) {
        selectedNum = 2;
        secondSelectedGemstone = gem;
        // 显示第二个选择框
        selectionRing2->setPosition(gem->transform()->translation());
        selectionRing2->setVisible(true);
        
        // 这里可以触发交换逻辑，但题目只要求选中状态
    }
    // selectedNum >= 2 时不做任何事
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
        } else if (event->type() == QEvent::MouseButtonPress) {
            refreshDebugStatus();
            appendDebug("container3d MouseButtonPress");
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
    game3dWindow->setRootEntity(rootEntity);
    
    // 配置 Picking 设置
    // 获取 Active FrameGraph 的 RenderSettings (如果存在)
    // 通常 Qt3DWindow 会自动设置一个默认的 RenderSettings，但我们需要确保 PickingSettings 正确
    Qt3DRender::QRenderSettings *renderSettings = game3dWindow->renderSettings();
    if (renderSettings) {
        renderSettings->pickingSettings()->setPickMethod(Qt3DRender::QPickingSettings::TrianglePicking);
        renderSettings->pickingSettings()->setFaceOrientationPickingMode(Qt3DRender::QPickingSettings::FrontAndBackFace);
        renderSettings->pickingSettings()->setPickResultMode(Qt3DRender::QPickingSettings::NearestPick);
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
    timer->start(16);  // 启动定时器（约60fps）
}

void SingleModeGameWidget::appendDebug(const QString& text) {
    if (!debugText) return;
    debugText->append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(text));
}

void SingleModeGameWidget::refreshDebugStatus() {
    if (!focusInfoLabel) return;
    bool hasFocusContainer = container3d ? container3d->hasFocus() : false;
    QWidget* active = QApplication::activeWindow();
    QString activeTitle = active ? active->windowTitle() : QString("null");
    focusInfoLabel->setText(QString("ContainerFocus=%1 | ActiveWindow=%2").arg(hasFocusContainer ? "true" : "false").arg(activeTitle));
}
