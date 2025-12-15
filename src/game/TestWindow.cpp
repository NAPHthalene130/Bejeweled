#include "TestWindow.h"
#include "components/Gemstone.h"
#include <QVBoxLayout>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DCore/QTransform>
#include <QVector3D>

TestWindow::TestWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("Gemstone Styles Test (3D)");
    resize(800, 600);

    // Initialize 3D Window
    view3D = new Qt3DExtras::Qt3DWindow();
    
    // Create container for 3D window
    container = QWidget::createWindowContainer(view3D);
    
    // Setup Layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(container);
    layout->setContentsMargins(0, 0, 0, 0);

    setup3DScene();
}

TestWindow::~TestWindow() {
    // Root entity cleanup handles children (gemstones, lights)
    if (view3D) {
        delete view3D;
    }
}

void TestWindow::setup3DScene() {
    // Root Entity
    rootEntity = new Qt3DCore::QEntity();
    view3D->setRootEntity(rootEntity);

    // Camera
    cameraEntity = view3D->camera();
    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0.0f, 0.0f, 15.0f));
    cameraEntity->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    // Light
    lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(1.0f);
    lightEntity->addComponent(light);
    
    Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(0.0f, 0.0f, 20.0f));
    lightEntity->addComponent(lightTransform);

    // Create 8 types of gemstones
    // Arrange in 2 rows of 4
    float startX = -3.0f;
    float startY = 1.5f;
    float spacingX = 2.0f;
    float spacingY = 3.0f;

    for (int i = 0; i < 8; ++i) {
        Gemstone* gem = new Gemstone(i, "default", rootEntity);
        gemstones.push_back(gem);

        int row = i / 4;
        int col = i % 4;

        float x = startX + col * spacingX;
        float y = startY - row * spacingY;

        gem->transform()->setTranslation(QVector3D(x, y, 0.0f));
    }
}
