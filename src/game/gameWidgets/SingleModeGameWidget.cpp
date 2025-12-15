#include "SingleModeGameWidget.h"
#include "../components/Gemstone.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <Qt3DExtras/Qt3DWindow>
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
    
    // 初始化3D窗口
    game3dWindow = new Qt3DExtras::Qt3DWindow();
    
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
    mainLayout->addStretch(1); // 将内容推向左侧
    
    setLayout(mainLayout);
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
    float startX = -3.5f * 1.5f; // 居中网格
    float startY = 3.5f * 1.5f;
    float spacing = 1.5f;
    
    for (int i = 0; i < 8; ++i) {
        gemstoneContainer[i].resize(8);
        for (int j = 0; j < 8; ++j) {
            int type = QRandomGenerator::global()->bounded(8);
            Gemstone* gem = new Gemstone(type, "default", rootEntity);
            
            // 设置位置：[0][0] 是左上角
            // i 是行 (Y), j 是列 (X)
            // 在3D中：X向右增加，Y向上增加。
            // 所以列 j 映射到 X，行 i 映射到 -Y（向下）
            
            float x = startX + j * spacing;
            float y = startY - i * spacing;
            
            gem->transform()->setTranslation(QVector3D(x, y, 0.0f));
            
            gemstoneContainer[i][j] = gem;
        }
    }
    
    // 重置定时器
    if (timer->isActive()) {
        timer->stop();
    }
}
