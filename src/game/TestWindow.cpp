#include "TestWindow.h"
#include "components/Gemstone.h"
#include <QVBoxLayout>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DCore/QTransform>
#include <QVector3D>

TestWindow::TestWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Gemstone 3D Test");
    resize(800, 600);
    
    // 布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    setup3DScene();
    
    if (container) {
        layout->addWidget(container);
    }
}

TestWindow::~TestWindow() {
    // Root entity cleanup handles children (gemstones, lights)
    if (view3D) {
        delete view3D;
    }
}

void TestWindow::setup3DScene() {
    // 初始化3D窗口
    view3D = new Qt3DExtras::Qt3DWindow();
    view3D->defaultFrameGraph()->setClearColor(QColor(30, 30, 30));
    
    // 创建3D窗口容器
    container = QWidget::createWindowContainer(view3D);
    
    // 根实体
    rootEntity = new Qt3DCore::QEntity();
    view3D->setRootEntity(rootEntity);
    
    // 相机
    Qt3DRender::QCamera *camera = view3D->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 0, 20.0f));
    camera->setViewCenter(QVector3D(0, 0, 0));
    
    // 灯光
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(1.0f);
    lightEntity->addComponent(light);
    
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(0, 0, 20.0f));
    lightEntity->addComponent(lightTransform);
    
    // 示例宝石
    // 在网格中显示所有8种类型
    int types = 8;
    float startX = -((types - 1) * 2.5f) / 2.0f;
    
    // Normal gems (top row)
    for (int i = 0; i < types; ++i) {
        Gemstone* gem = new Gemstone(i, "default", rootEntity);
        gem->transform()->setTranslation(QVector3D(startX + i * 2.5f, 2.0f, 0));
    }

    // Special gems (bottom row)
    for (int i = 0; i < types; ++i) {
        Gemstone* gem = new Gemstone(i, "default", rootEntity);
        gem->transform()->setTranslation(QVector3D(startX + i * 2.5f, -2.0f, 0));
        gem->setSpecial(true);
    }
}
