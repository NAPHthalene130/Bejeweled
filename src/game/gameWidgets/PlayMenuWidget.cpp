#include "PlayMenuWidget.h"
#include "../GameWindow.h"
#include "../components/MenuButton.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QApplication>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DExtras/qforwardrenderer.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>
#include <Qt3DRender/qcamera.h>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QMetalRoughMaterial>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QPointLight>
#include <QVector3D>
#include <QRandomGenerator>

PlayMenuWidget::PlayMenuWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    setupUI();
    setup3DView();
}

void PlayMenuWidget::setupUI() {
    setMinimumSize(1600, 1000);
    
    // Main layout with center alignment
    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(30);

    // Button Configuration
    int btnW = 300;
    int btnH = 70;
    int fontSize = 22;

    // Button 1: Normal Mode
    normalModeButton = new MenuButton(btnW, btnH, fontSize, QColor(100, 149, 237), "单人模式-原版", this); // CornflowerBlue
    normalModeButton->setAttribute(Qt::WA_NativeWindow);

    // Button 2: Rotate Mode
    rotateModeButton = new MenuButton(btnW, btnH, fontSize, QColor(147, 112, 219), "单人模式-旋风", this); // MediumPurple
    rotateModeButton->setAttribute(Qt::WA_NativeWindow);

    // Button 3: Multi Mode
    multiModeButton = new MenuButton(btnW, btnH, fontSize, QColor(255, 127, 80), "多人模式-对战", this); // Coral
    multiModeButton->setAttribute(Qt::WA_NativeWindow);

    // Back Button (Small, Top Left or Bottom)
    backButton = new MenuButton(150, 50, 16, QColor(200, 200, 200), "返回", this);
    backButton->setAttribute(Qt::WA_NativeWindow);

    // Add buttons to layout
    mainLayout->addStretch(1);
    mainLayout->addWidget(normalModeButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(rotateModeButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(multiModeButton, 0, Qt::AlignCenter);
    mainLayout->addStretch(1);
    
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);
    mainLayout->addSpacing(50);

    connect(backButton, &QPushButton::clicked, this, [this]() {
        emit backToMenu();
    });
}

void PlayMenuWidget::setup3DView() {
    view3D = new Qt3DExtras::Qt3DWindow();
    
    // Gray Theme Background
    view3D->defaultFrameGraph()->setClearColor(QColor(40, 40, 45)); // Dark Gray
    rootEntity = new Qt3DCore::QEntity();
    view3D->setRootEntity(rootEntity);

    auto camera = view3D->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0.0f, 0.0f, 20.0f));
    camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    // Lights
    Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(1.5f);
    lightEntity->addComponent(light);
    auto lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(0.0f, 10.0f, 10.0f));
    lightEntity->addComponent(lightTransform);

    // --- Cool Effect: Rotating Rings of Cubes ---
    
    auto createRing = [&](float radius, int count, float speed, float yOffset, const QColor& color) {
        auto ringRoot = new Qt3DCore::QEntity(rootEntity);
        auto ringRootTransform = new Qt3DCore::QTransform();
        ringRoot->addComponent(ringRootTransform);

        // Animate the whole ring rotating
        auto anim = new QPropertyAnimation(ringRootTransform, "rotationY");
        anim->setStartValue(0.0f);
        anim->setEndValue(360.0f);
        anim->setDuration(speed);
        anim->setLoopCount(-1);
        anim->start();

        // Add cubes
        auto mesh = new Qt3DExtras::QCuboidMesh();
        mesh->setXExtent(1.0f); mesh->setYExtent(1.0f); mesh->setZExtent(1.0f);

        auto mat = new Qt3DExtras::QPhongMaterial();
        mat->setDiffuse(color);
        mat->setAmbient(color.darker());
        mat->setSpecular(Qt::white);
        mat->setShininess(100.0f);

        for (int i = 0; i < count; ++i) {
            auto objEntity = new Qt3DCore::QEntity(ringRoot);
            auto transform = new Qt3DCore::QTransform();
            
            float angle = (360.0f / count) * i;
            float rad = qDegreesToRadians(angle);
            float x = radius * cos(rad);
            float z = radius * sin(rad);

            transform->setTranslation(QVector3D(x, yOffset, z));
            // Random tumbling
            transform->setRotationX(QRandomGenerator::global()->bounded(360));
            transform->setRotationZ(QRandomGenerator::global()->bounded(360));

            objEntity->addComponent(mesh);
            objEntity->addComponent(mat);
            objEntity->addComponent(transform);
        }
    };

    // Create a few rings
    createRing(8.0f, 12, 8000, 0.0f, QColor(0, 255, 255)); // Outer Cyan Ring
    createRing(5.0f, 8, -6000, 2.0f, QColor(255, 0, 255)); // Middle Magenta Ring
    createRing(3.0f, 6, 4000, -2.0f, QColor(255, 165, 0));   // Inner Orange Ring

    // Central Object - Large Rotating Icosahedron/Sphere
    auto centerEntity = new Qt3DCore::QEntity(rootEntity);
    auto centerMesh = new Qt3DExtras::QSphereMesh();
    centerMesh->setRadius(1.5f);
    centerMesh->setRings(20);
    centerMesh->setSlices(20);

    auto centerMat = new Qt3DExtras::QPhongMaterial();
    centerMat->setDiffuse(QColor(0, 200, 255)); // Light Blue
    centerMat->setAmbient(QColor(0, 50, 100));
    centerMat->setSpecular(QColor(255, 255, 255));
    
    auto centerTransform = new Qt3DCore::QTransform();
    centerEntity->addComponent(centerMesh);
    centerEntity->addComponent(centerMat);
    centerEntity->addComponent(centerTransform);

    // Animate center
    auto centerAnim = new QPropertyAnimation(centerTransform, "scale3D");
    centerAnim->setStartValue(QVector3D(1.0f, 1.0f, 1.0f));
    centerAnim->setEndValue(QVector3D(1.2f, 1.2f, 1.2f));
    centerAnim->setDuration(2000);
    centerAnim->setLoopCount(-1);
    centerAnim->setEasingCurve(QEasingCurve::SineCurve);
    centerAnim->start();

    // Container
    view3DContainer = QWidget::createWindowContainer(view3D, this);
    view3DContainer->setMinimumSize(0, 0);
    view3DContainer->lower();
}

void PlayMenuWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (view3DContainer) {
        view3DContainer->setGeometry(rect());
        view3DContainer->lower();
    }
}
