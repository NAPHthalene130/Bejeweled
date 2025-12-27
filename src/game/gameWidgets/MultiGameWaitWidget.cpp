#include "MultiGameWaitWidget.h"
#include "PlayMenuWidget.h"
#include "../GameWindow.h"
#include "../data/NetDataIO.h"
#include "../data/GameNetData.h"
#include "../components/MenuButton.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QRandomGenerator>
#include <QtMath>
#include <cmath>
#include <algorithm>

// Qt3D Includes
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QMetalRoughMaterial>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QPointLight>

MultiGameWaitWidget::MultiGameWaitWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow), isInRoom(false), roomPeopleHave(0) {
    setupUI();
    setup3DView();
}

void MultiGameWaitWidget::enterRoom() {
    roomPeopleHave = 0;
    updateInfoLabel();
    if (gameWindow) {
        gameWindow->switchWidget(this);
    }
}

MultiGameWaitWidget::~MultiGameWaitWidget() {
    // Qt3DWindow is managed by the container, but explicit cleanup can be safer if needed.
    // Usually standard Qt parent-child cleanup works.
}

bool MultiGameWaitWidget::getIsInRoom() const {
    return isInRoom;
}

void MultiGameWaitWidget::setIsInRoom(bool value) {
    isInRoom = value;
    // Potentially update UI or 3D scene based on room status
}

int MultiGameWaitWidget::getRoomPeopleHave() const {
    return roomPeopleHave;
}

void MultiGameWaitWidget::setRoomPeopleHave(int count) {
    roomPeopleHave = count;
    updateInfoLabel();
}

void MultiGameWaitWidget::resetRoomPeopleHaveLabel(int people) {
    setRoomPeopleHave(people);
}

void MultiGameWaitWidget::backButtonClicked() {
    if (gameWindow) {
        NetDataIO* netDataIO = gameWindow->getNetDataIO();
        if (netDataIO) {
            GameNetData gameNetData;
            gameNetData.setType(0);
            gameNetData.setData("EXIT");
            gameNetData.setID(gameWindow->getUserID());
            netDataIO->sendData(gameNetData);
        }

        if (gameWindow->getPlayMenuWidget()) {
             // 切换回PlayMenuWidget
             gameWindow->switchWidget(gameWindow->getPlayMenuWidget());
        }
    }
}

void MultiGameWaitWidget::updateInfoLabel() {
    if (infoLabel) {
        infoLabel->setText(QString("当前玩家人数: %1 人").arg(roomPeopleHave));
    }
}

void MultiGameWaitWidget::setupUI() {
    setMinimumSize(1200, 800);

    // Main layout for the widget
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->setSpacing(40);
    mainLayout->setContentsMargins(50, 0, 0, 0);

    // Info Label
    infoLabel = new QLabel(this); 
    infoLabel->setAttribute(Qt::WA_NativeWindow); // Ensure it's on top of native 3D window
    infoLabel->setAlignment(Qt::AlignCenter);
    updateInfoLabel();
    
    // Style for the label: Glassmorphism / HUD look
    infoLabel->setStyleSheet(
        "QLabel {"
        "   color: #00ffff;"
        "   font-family: 'Segoe UI', sans-serif;"
        "   font-size: 36px;"
        "   font-weight: bold;"
        "   background-color: rgba(10, 20, 40, 180);"
        "   border: 2px solid rgba(0, 255, 255, 100);"
        "   border-radius: 0px;"
        "   padding: 20px 60px;"
        "}"
    );
    
    mainLayout->addWidget(infoLabel, 0, Qt::AlignHCenter);
    
    // Back Button
    backButton = new MenuButton(200, 60, 30, QColor(255, 80, 80), "返回菜单", this);
    backButton->setAttribute(Qt::WA_NativeWindow); // Ensure it's on top of native 3D window
    connect(backButton, &QPushButton::clicked, this, &MultiGameWaitWidget::backButtonClicked);
    
    mainLayout->addWidget(backButton, 0, Qt::AlignHCenter);

    // Ensure widgets are raised (though NativeWindow attribute helps most)
    infoLabel->raise();
    backButton->raise();
}

void MultiGameWaitWidget::setup3DView() {
    // Create 3D Window
    view3D = new Qt3DExtras::Qt3DWindow();
    view3D->defaultFrameGraph()->setClearColor(QColor(5, 5, 10));

    // Container
    view3DContainer = QWidget::createWindowContainer(view3D, this);
    view3DContainer->setMinimumSize(0, 0);
    // Geometry will be set in resizeEvent
    view3DContainer->lower(); // Send to back
    view3DContainer->show();

    // Scene Root
    rootEntity = new Qt3DCore::QEntity();
    view3D->setRootEntity(rootEntity);

    // Camera
    auto camera = view3D->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    
    // Position camera to see objects, but look slightly to the left (-8, 0, 0) 
    // so that the objects (at 0,0,0) appear shifted to the right on screen.
    camera->setPosition(QVector3D(0.0f, 10.0f, 25.0f));
    camera->setViewCenter(QVector3D(-8.0f, 0.0f, 0.0f));

    // --- Lighting ---
    auto* lightEntity = new Qt3DCore::QEntity(rootEntity);
    auto* pointLight = new Qt3DRender::QPointLight(lightEntity);
    pointLight->setColor(QColor(200, 255, 255));
    pointLight->setIntensity(1.5f);
    lightEntity->addComponent(pointLight);
    auto* lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(0.0f, 15.0f, 10.0f));
    lightEntity->addComponent(lightTransform);

    // --- Central Core (The "Server") ---
    // A pulsing icy sphere
    auto* coreEntity = new Qt3DCore::QEntity(rootEntity);
    auto* coreMesh = new Qt3DExtras::QSphereMesh(coreEntity);
    coreMesh->setRadius(2.5f);
    coreMesh->setRings(64);
    coreMesh->setSlices(64);
    
    auto* coreMat = new Qt3DExtras::QMetalRoughMaterial(coreEntity);
    coreMat->setBaseColor(QColor(0, 200, 255));
    coreMat->setMetalness(0.2f);
    coreMat->setRoughness(0.1f);
    
    auto* coreTransform = new Qt3DCore::QTransform();
    coreEntity->addComponent(coreMesh);
    coreEntity->addComponent(coreMat);
    coreEntity->addComponent(coreTransform);

    // Pulse Animation for Core
    auto* pulseAnim = new QPropertyAnimation(coreTransform, "scale3D", this);
    pulseAnim->setStartValue(QVector3D(1.0f, 1.0f, 1.0f));
    pulseAnim->setEndValue(QVector3D(1.1f, 1.1f, 1.1f));
    pulseAnim->setDuration(1500);
    pulseAnim->setLoopCount(-1);
    pulseAnim->setEasingCurve(QEasingCurve::InOutSine);
    pulseAnim->start();

    // --- Rotating Rings (Data Streams) ---
    auto createRing = [&](float radius, float tubeRadius, const QColor& color, const QVector3D& axis, int duration) {
        auto* ringEntity = new Qt3DCore::QEntity(rootEntity);
        auto* ringMesh = new Qt3DExtras::QTorusMesh(ringEntity);
        ringMesh->setRadius(radius);
        ringMesh->setMinorRadius(tubeRadius);
        ringMesh->setRings(64);
        ringMesh->setSlices(32);

        auto* ringMat = new Qt3DExtras::QPhongMaterial(ringEntity);
        ringMat->setDiffuse(color);
        ringMat->setAmbient(color.darker(200));
        ringMat->setSpecular(Qt::white);
        ringMat->setShininess(100.0f);
        // ringMat->setOpacity(0.8f); // setOpacity not available in basic QPhongMaterial in some versions


        auto* ringTransform = new Qt3DCore::QTransform();
        ringTransform->setRotation(QQuaternion::fromAxisAndAngle(axis, 0.0f));

        ringEntity->addComponent(ringMesh);
        ringEntity->addComponent(ringMat);
        ringEntity->addComponent(ringTransform);

        auto* rotAnim = new QPropertyAnimation(ringTransform, "rotation", this);
        rotAnim->setStartValue(QQuaternion::fromAxisAndAngle(axis, 0.0f));
        rotAnim->setEndValue(QQuaternion::fromAxisAndAngle(axis, 360.0f));
        rotAnim->setDuration(duration);
        rotAnim->setLoopCount(-1);
        rotAnim->start();
    };

    createRing(4.5f, 0.1f, QColor(0, 255, 255), QVector3D(1, 0, 0), 8000);
    createRing(5.5f, 0.15f, QColor(0, 100, 255), QVector3D(0, 1, 0), 12000);
    createRing(6.5f, 0.08f, QColor(255, 0, 255), QVector3D(0, 0, 1), 15000);

    // --- Orbiting "Players" (Cubes) ---
    auto* orbitRoot = new Qt3DCore::QEntity(rootEntity);
    auto* orbitRootTransform = new Qt3DCore::QTransform();
    orbitRoot->addComponent(orbitRootTransform);
    
    // Rotate the whole orbit system slowly
    auto* systemAnim = new QPropertyAnimation(orbitRootTransform, "rotationY", this);
    systemAnim->setStartValue(0.0f);
    systemAnim->setEndValue(360.0f);
    systemAnim->setDuration(30000);
    systemAnim->setLoopCount(-1);
    systemAnim->start();

    int playerCount = 12;
    for (int i = 0; i < playerCount; ++i) {
        auto* playerEntity = new Qt3DCore::QEntity(orbitRoot);
        auto* playerMesh = new Qt3DExtras::QCuboidMesh(playerEntity);
        playerMesh->setXExtent(0.8f);
        playerMesh->setYExtent(0.8f);
        playerMesh->setZExtent(0.8f);

        auto* playerMat = new Qt3DExtras::QPhongMaterial(playerEntity);
        double hue = (360.0 / playerCount) * i;
        playerMat->setDiffuse(QColor::fromHsvF(hue/360.0, 0.8, 0.9));
        
        auto* playerTransform = new Qt3DCore::QTransform();
        float angle = (2.0 * M_PI / playerCount) * i;
        float radius = 9.0f + std::sin(angle * 3) * 2.0f; // Wobbly orbit
        float y = std::cos(angle * 2) * 3.0f;
        
        playerTransform->setTranslation(QVector3D(radius * std::cos(angle), y, radius * std::sin(angle)));
        
        playerEntity->addComponent(playerMesh);
        playerEntity->addComponent(playerMat);
        playerEntity->addComponent(playerTransform);

        // Spin individual cubes
        auto* spinAnim = new QPropertyAnimation(playerTransform, "rotation", this);
        QVector3D axis(
            QRandomGenerator::global()->generateDouble(), 
            QRandomGenerator::global()->generateDouble(), 
            QRandomGenerator::global()->generateDouble()
        );
        spinAnim->setStartValue(QQuaternion::fromAxisAndAngle(axis, 0.0f));
        spinAnim->setEndValue(QQuaternion::fromAxisAndAngle(axis, 360.0f));
        spinAnim->setDuration(2000 + QRandomGenerator::global()->bounded(3000));
        spinAnim->setLoopCount(-1);
        spinAnim->start();
    }

    // --- Floating Particles (Data Bits) ---
    // Using many small spheres
    int particleCount = 60;
    for (int i = 0; i < particleCount; ++i) {
        auto* pEntity = new Qt3DCore::QEntity(rootEntity);
        auto* pMesh = new Qt3DExtras::QSphereMesh(pEntity);
        pMesh->setRadius(0.15f);
        
        auto* pMat = new Qt3DExtras::QPhongMaterial(pEntity);
        pMat->setDiffuse(QColor(200, 255, 200));
        pMat->setAmbient(Qt::black);
        
        auto* pTransform = new Qt3DCore::QTransform();
        float range = 20.0f;
        float x = -range/2 + QRandomGenerator::global()->generateDouble() * range;
        float y = -range/2 + QRandomGenerator::global()->generateDouble() * range;
        float z = -range/2 + QRandomGenerator::global()->generateDouble() * range;
        pTransform->setTranslation(QVector3D(x, y, z));
        
        pEntity->addComponent(pMesh);
        pEntity->addComponent(pMat);
        pEntity->addComponent(pTransform);

        // Simple floating animation (vertical bobbing)
        // Using QVariantAnimation to drive the Y offset for smooth ping-pong effect
        auto* varAnim = new QVariantAnimation(this);
        varAnim->setStartValue(0.0f);
        varAnim->setEndValue(1.0f);
        varAnim->setDuration(3000 + QRandomGenerator::global()->bounded(2000));
        varAnim->setLoopCount(-1);
        connect(varAnim, &QVariantAnimation::valueChanged, this, [pTransform, x, y, z](const QVariant& v) {
            float t = v.toFloat();
            float offset = std::sin(t * 2 * M_PI) * 1.5f;
            pTransform->setTranslation(QVector3D(x, y + offset, z));
        });
        varAnim->start();
    }
}

// Override resizeEvent to handle the container resizing
void MultiGameWaitWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    
    // 3D View fills the whole window
    if (view3DContainer) {
        view3DContainer->setGeometry(rect());
        view3DContainer->lower();
    }
}
