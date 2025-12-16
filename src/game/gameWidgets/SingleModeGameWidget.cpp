#include "SingleModeGameWidget.h"
#include "../components/Gemstone.h"
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

SingleModeGameWidget::SingleModeGameWidget(QWidget* parent, GameWindow* gameWindow) 
    : QWidget(parent), gameWindow(gameWindow), canOpe(true), nowTimeHave(0), mode(1) {
    
    // 初始化定时器
    timer = new QTimer(this);
    
    // 设置主背景颜色
    setStyleSheet("background-color: rgb(40, 40, 45);");

    // 初始化3D窗口
    game3dWindow = new Qt3DExtras::Qt3DWindow();
    game3dWindow->defaultFrameGraph()->setClearColor(QColor(40, 40, 45)); // 深灰色背景
    
    // 设置3D场景
    setup3DScene();
    
    // 创建3D窗口容器
    container3d = QWidget::createWindowContainer(game3dWindow);
    container3d->setFixedSize(960, 960); 
    
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
    
    rightLayout->addWidget(backButton);
    rightLayout->addWidget(testEliminateButton);
    rightLayout->addWidget(testSwitchButton);
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

SingleModeGameWidget::~SingleModeGameWidget() {
    if (rootEntity) {
        delete rootEntity;
    }
    if (game3dWindow) {
        delete game3dWindow;
    }
}

void SingleModeGameWidget::setup3DScene() {
    // 根实体
    rootEntity = new Qt3DCore::QEntity();
    game3dWindow->setRootEntity(rootEntity);
    
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
            
            gemstoneContainer[i][j] = gem;
        }
    }
    
    // 重置定时器
    if (timer->isActive()) {
        timer->stop();
    }
}
