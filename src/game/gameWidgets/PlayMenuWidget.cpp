#include "PlayMenuWidget.h"
#include "../GameWindow.h"
#include "SingleModeGameWidget.h"
#include "MultiplayerModeGameWidget.h"
#include "MultiGameWaitWidget.h"
#include "../components/MenuButton.h"
#include "../data/GameNetData.h"
#include "../../auth/components/AuthNoticeDialog.h"
#include "../data/NetDataIO.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QResizeEvent>
#include <QApplication>
#include <QtMath>
#include <cmath>
#include <algorithm>
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
#include <QTcpSocket>
#include <QHostAddress>
#include <json.hpp>


PlayMenuWidget::PlayMenuWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    setupUI();
    setup3DView();
}

void PlayMenuWidget::setupUI() {
    setMinimumSize(1600, 1000);
    
    // 居中对齐的主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(30);

    // 按钮配置
    int btnW = 300;
    int btnH = 70;
    int fontSize = 22;

    // 按钮 1：普通模式
    normalModeButton = new MenuButton(btnW, btnH, fontSize, QColor(100, 149, 237), "单人模式-原版", this); // 矢车菊蓝
    normalModeButton->setAttribute(Qt::WA_NativeWindow);

    // 按钮 2：旋转模式
    rotateModeButton = new MenuButton(btnW, btnH, fontSize, QColor(147, 112, 219), "单人模式-无尽旋风", this); // 中紫色
    rotateModeButton->setAttribute(Qt::WA_NativeWindow);

    // 按钮 3：多人模式
    multiModeButton = new MenuButton(btnW, btnH, fontSize, QColor(255, 127, 80), "多人模式-对战", this); // 珊瑚色
    multiModeButton->setAttribute(Qt::WA_NativeWindow);

    // 按钮 4：多人测试
    testMultiButton = new MenuButton(btnW, btnH, fontSize, QColor(60, 179, 113), "多人测试", this); // MediumSeaGreen
    testMultiButton->setAttribute(Qt::WA_NativeWindow);

    //按钮5：解密模式
    puzzleModeButton= new MenuButton(btnW, btnH, fontSize, QColor(255, 215, 0), "解密模式", this); // 金色
    puzzleModeButton->setAttribute(Qt::WA_NativeWindow);


    // 返回按钮（小型，左上角或底部）
    backButton = new MenuButton(150, 50, 16, QColor(200, 200, 200), "返回", this);
    backButton->setAttribute(Qt::WA_NativeWindow);

    // 将按钮添加到布局
    mainLayout->addStretch(1);
    mainLayout->addWidget(normalModeButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(rotateModeButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(multiModeButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(testMultiButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(puzzleModeButton, 0, Qt::AlignCenter);
    mainLayout->addStretch(1);
    
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);
    mainLayout->addSpacing(50);

    connect(backButton, &QPushButton::clicked, this, [this]() {
        emit backToMenu();
    });

    connect(normalModeButton, &QPushButton::clicked, this, [this]() {
        if (gameWindow && gameWindow->getSingleModeGameWidget()) {
            gameWindow->getSingleModeGameWidget()->reset(1);
        }
        emit startNormalMode();
    });

    connect(rotateModeButton, &QPushButton::clicked, this, [this]() {
        if (gameWindow && gameWindow->getSingleModeGameWidget()) {
            gameWindow->getSingleModeGameWidget()->reset(2);
        }
        emit startRotateMode();
    });

    connect(multiModeButton, &QPushButton::clicked, this, &PlayMenuWidget::multiModeButtonClicked);

    connect(testMultiButton, &QPushButton::clicked, this, [this]() {
        if (gameWindow && gameWindow->getMultiplayerModeGameWidget()) {
            gameWindow->getMultiplayerModeGameWidget()->reset(1);  // Normal mode for multiplayer
            gameWindow->switchWidget(gameWindow->getMultiplayerModeGameWidget());
        }
    });
}
    

void PlayMenuWidget::setup3DView() {
    view3D = new Qt3DExtras::Qt3DWindow();
    
    auto* renderer = view3D->defaultFrameGraph();
    renderer->setClearColor(QColor(10, 10, 25));
    rootEntity = new Qt3DCore::QEntity();
    view3D->setRootEntity(rootEntity);

    auto camera = view3D->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0.0f, 0.0f, 20.0f));
    camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    auto* bgAnim = new QVariantAnimation(this);
    bgAnim->setStartValue(0.0);
    bgAnim->setEndValue(1.0);
    bgAnim->setDuration(16000);
    bgAnim->setLoopCount(-1);
    connect(bgAnim, &QVariantAnimation::valueChanged, this, [renderer](const QVariant& v) {
        constexpr double TwoPi = 6.2831853071795864769;
        const double t = v.toDouble();
        const double val = std::clamp(0.06 + 0.18 * (0.5 + 0.5 * std::sin(TwoPi * t * 0.55 + 1.1)), 0.0, 1.0);
        renderer->setClearColor(QColor::fromRgbF(val, val, val));
    });
    bgAnim->start();

    Qt3DCore::QEntity* dirLightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QDirectionalLight* dirLight = new Qt3DRender::QDirectionalLight(dirLightEntity);
    dirLight->setColor(Qt::white);
    dirLight->setIntensity(0.28f);
    dirLight->setWorldDirection(QVector3D(-0.3f, -1.0f, -0.6f));
    dirLightEntity->addComponent(dirLight);

    auto createOrbitingLight = [&](float radius, float y, float z, int durationMs, float minIntensity, float maxIntensity) {
        auto* orbitRoot = new Qt3DCore::QEntity(rootEntity);
        auto* orbitTransform = new Qt3DCore::QTransform();
        orbitRoot->addComponent(orbitTransform);

        auto* orbitAnim = new QPropertyAnimation(orbitTransform, "rotationY", this);
        orbitAnim->setStartValue(0.0f);
        orbitAnim->setEndValue(360.0f);
        orbitAnim->setDuration(durationMs);
        orbitAnim->setLoopCount(-1);
        orbitAnim->start();

        auto* lightEntity = new Qt3DCore::QEntity(orbitRoot);
        auto* pointLight = new Qt3DRender::QPointLight(lightEntity);
        pointLight->setColor(Qt::white);
        pointLight->setIntensity(maxIntensity);
        lightEntity->addComponent(pointLight);

        auto* lightTransform = new Qt3DCore::QTransform();
        lightTransform->setTranslation(QVector3D(radius, y, z));
        lightEntity->addComponent(lightTransform);

        auto* intensityAnim = new QVariantAnimation(this);
        intensityAnim->setStartValue(0.0);
        intensityAnim->setEndValue(1.0);
        intensityAnim->setDuration(std::max(4200, durationMs / 2));
        intensityAnim->setLoopCount(-1);
        connect(intensityAnim, &QVariantAnimation::valueChanged, this, [pointLight, minIntensity, maxIntensity](const QVariant& v) {
            constexpr double TwoPi = 6.2831853071795864769;
            const double t = v.toDouble();
            const double k = 0.5 + 0.5 * std::sin(TwoPi * t);
            pointLight->setIntensity(minIntensity + static_cast<float>((maxIntensity - minIntensity) * k));
        });
        intensityAnim->start();
    };

    createOrbitingLight(7.0f, 4.0f, 10.0f, 9000, 0.4f, 0.8f);
    createOrbitingLight(7.0f, -3.5f, 9.0f, 12000, 0.3f, 0.7f);
    createOrbitingLight(6.0f, 0.0f, 11.0f, 7000, 0.3f, 0.6f);

    // --- 炫酷效果：旋转的立方体环 ---
    
    auto createRing = [&](float radius, int count, int durationMs, float yOffset, bool clockwise) {
        auto ringRoot = new Qt3DCore::QEntity(rootEntity);
        auto ringRootTransform = new Qt3DCore::QTransform();
        ringRoot->addComponent(ringRootTransform);

        auto anim = new QPropertyAnimation(ringRootTransform, "rotationY", this);
        anim->setStartValue(0.0f);
        anim->setEndValue(clockwise ? 360.0f : -360.0f);
        anim->setDuration(durationMs);
        anim->setLoopCount(-1);
        anim->start();

        auto* cubeMesh = new Qt3DExtras::QCuboidMesh(ringRoot);
        cubeMesh->setXExtent(0.9f);
        cubeMesh->setYExtent(0.9f);
        cubeMesh->setZExtent(0.9f);

        auto* cylMesh = new Qt3DExtras::QCylinderMesh(ringRoot);
        cylMesh->setLength(1.1f);
        cylMesh->setRadius(0.38f);
        cylMesh->setRings(32);
        cylMesh->setSlices(32);

        auto* torusMesh = new Qt3DExtras::QTorusMesh(ringRoot);
        torusMesh->setRadius(0.6f);
        torusMesh->setMinorRadius(0.16f);
        torusMesh->setRings(48);
        torusMesh->setSlices(28);

        for (int i = 0; i < count; ++i) {
            auto objEntity = new Qt3DCore::QEntity(ringRoot);
            auto transform = new Qt3DCore::QTransform();
            
            float angle = (360.0f / count) * i;
            float rad = qDegreesToRadians(angle);
            float x = radius * cos(rad);
            float z = radius * sin(rad);

            transform->setTranslation(QVector3D(x, yOffset, z));
            transform->setRotationX(QRandomGenerator::global()->bounded(360));
            transform->setRotationZ(QRandomGenerator::global()->bounded(360));

            auto* mat = new Qt3DExtras::QPhongMaterial(objEntity);
            const double t = static_cast<double>(i) / static_cast<double>(std::max(1, count - 1));
            const double hue = 190.0 + 110.0 * t;
            const QColor c = QColor::fromHsvF(hue / 360.0, 0.75, 0.82);
            mat->setDiffuse(c);
            mat->setAmbient(c.darker(260));
            mat->setSpecular(QColor(255, 255, 255));
            mat->setShininess(140.0f);

            if (i % 3 == 0) objEntity->addComponent(cubeMesh);
            else if (i % 3 == 1) objEntity->addComponent(cylMesh);
            else objEntity->addComponent(torusMesh);

            objEntity->addComponent(transform);
            objEntity->addComponent(mat);

            auto* spin = new QPropertyAnimation(transform, "rotationY", this);
            spin->setStartValue(QRandomGenerator::global()->bounded(360));
            spin->setEndValue(QRandomGenerator::global()->bounded(360) + (clockwise ? 360.0f : -360.0f));
            spin->setDuration(QRandomGenerator::global()->bounded(2400, 5200));
            spin->setLoopCount(-1);
            spin->setEasingCurve(QEasingCurve::InOutSine);
            spin->start();
        }
    };

    createRing(8.2f, 18, 9000, 0.2f, true);
    createRing(5.6f, 14, 6800, 2.2f, false);
    createRing(3.5f, 10, 5200, -2.0f, true);

    auto* halo1 = new Qt3DCore::QEntity(rootEntity);
    auto* haloMesh1 = new Qt3DExtras::QTorusMesh(halo1);
    haloMesh1->setRadius(2.7f);
    haloMesh1->setMinorRadius(0.06f);
    haloMesh1->setRings(96);
    haloMesh1->setSlices(48);
    auto* haloMat1 = new Qt3DExtras::QPhongMaterial(halo1);
    haloMat1->setDiffuse(QColor(85, 150, 210));
    haloMat1->setAmbient(QColor(10, 20, 35));
    haloMat1->setSpecular(QColor(255, 255, 255));
    haloMat1->setShininess(200.0f);
    auto* haloTr1 = new Qt3DCore::QTransform();
    haloTr1->setRotationX(60.0f);
    halo1->addComponent(haloMesh1);
    halo1->addComponent(haloMat1);
    halo1->addComponent(haloTr1);
    auto* haloAnim1 = new QPropertyAnimation(haloTr1, "rotationY", this);
    haloAnim1->setStartValue(0.0f);
    haloAnim1->setEndValue(360.0f);
    haloAnim1->setDuration(12000);
    haloAnim1->setLoopCount(-1);
    haloAnim1->start();

    auto* halo2 = new Qt3DCore::QEntity(rootEntity);
    auto* haloMesh2 = new Qt3DExtras::QTorusMesh(halo2);
    haloMesh2->setRadius(2.2f);
    haloMesh2->setMinorRadius(0.05f);
    haloMesh2->setRings(96);
    haloMesh2->setSlices(48);
    auto* haloMat2 = new Qt3DExtras::QPhongMaterial(halo2);
    haloMat2->setDiffuse(QColor(135, 105, 210));
    haloMat2->setAmbient(QColor(20, 10, 35));
    haloMat2->setSpecular(QColor(255, 255, 255));
    haloMat2->setShininess(190.0f);
    auto* haloTr2 = new Qt3DCore::QTransform();
    haloTr2->setRotationZ(75.0f);
    halo2->addComponent(haloMesh2);
    halo2->addComponent(haloMat2);
    halo2->addComponent(haloTr2);
    auto* haloAnim2 = new QPropertyAnimation(haloTr2, "rotationY", this);
    haloAnim2->setStartValue(0.0f);
    haloAnim2->setEndValue(-360.0f);
    haloAnim2->setDuration(15000);
    haloAnim2->setLoopCount(-1);
    haloAnim2->start();

    auto* coreEntity = new Qt3DCore::QEntity(rootEntity);
    auto* coreMesh = new Qt3DExtras::QCuboidMesh(coreEntity);
    coreMesh->setXExtent(1.2f);
    coreMesh->setYExtent(1.2f);
    coreMesh->setZExtent(1.2f);
    auto* coreMat = new Qt3DExtras::QMetalRoughMaterial(coreEntity);
    coreMat->setBaseColor(QColor(65, 160, 200));
    coreMat->setMetalness(0.9f);
    coreMat->setRoughness(0.15f);
    auto* coreTransform = new Qt3DCore::QTransform();
    coreEntity->addComponent(coreMesh);
    coreEntity->addComponent(coreMat);
    coreEntity->addComponent(coreTransform);
    auto* coreRotY = new QPropertyAnimation(coreTransform, "rotationY", this);
    coreRotY->setStartValue(0.0f);
    coreRotY->setEndValue(360.0f);
    coreRotY->setDuration(4200);
    coreRotY->setLoopCount(-1);
    coreRotY->setEasingCurve(QEasingCurve::InOutSine);
    coreRotY->start();
    auto* coreRotX = new QPropertyAnimation(coreTransform, "rotationX", this);
    coreRotX->setStartValue(0.0f);
    coreRotX->setEndValue(-360.0f);
    coreRotX->setDuration(5600);
    coreRotX->setLoopCount(-1);
    coreRotX->setEasingCurve(QEasingCurve::InOutSine);
    coreRotX->start();

    auto* centerEntity = new Qt3DCore::QEntity(rootEntity);
    auto* centerMesh = new Qt3DExtras::QSphereMesh(centerEntity);
    centerMesh->setRadius(1.55f);
    centerMesh->setRings(24);
    centerMesh->setSlices(24);
    auto* centerMat = new Qt3DExtras::QPhongMaterial(centerEntity);
    centerMat->setDiffuse(QColor(70, 150, 205));
    centerMat->setAmbient(QColor(10, 18, 35));
    centerMat->setSpecular(QColor(255, 255, 255));
    centerMat->setShininess(180.0f);
    auto* centerTransform = new Qt3DCore::QTransform();
    centerEntity->addComponent(centerMesh);
    centerEntity->addComponent(centerMat);
    centerEntity->addComponent(centerTransform);

    auto* centerScale = new QPropertyAnimation(centerTransform, "scale3D", this);
    centerScale->setStartValue(QVector3D(0.95f, 0.95f, 0.95f));
    centerScale->setEndValue(QVector3D(1.25f, 1.25f, 1.25f));
    centerScale->setDuration(2000);
    centerScale->setLoopCount(-1);
    centerScale->setEasingCurve(QEasingCurve::InOutSine);
    centerScale->start();

    auto* centerColor = new QVariantAnimation(this);
    centerColor->setStartValue(0.0);
    centerColor->setEndValue(1.0);
    centerColor->setDuration(9000);
    centerColor->setLoopCount(-1);
    connect(centerColor, &QVariantAnimation::valueChanged, this, [centerMat](const QVariant& v) {
        constexpr double TwoPi = 6.2831853071795864769;
        const double t = v.toDouble();
        const double k = 0.5 + 0.5 * std::sin(TwoPi * t);
        const double hue = 200.0 + 80.0 * k;
        centerMat->setDiffuse(QColor::fromHsvF(hue / 360.0, 0.45, 0.85));
    });
    centerColor->start();

    auto* starsRoot = new Qt3DCore::QEntity(rootEntity);

    const int starCount = 90;
    for (int i = 0; i < starCount; ++i) {
        auto* star = new Qt3DCore::QEntity(starsRoot);
        auto* tr = new Qt3DCore::QTransform();

        auto* starMesh = new Qt3DExtras::QSphereMesh(star);
        starMesh->setRings(12);
        starMesh->setSlices(12);
        const float baseRadius = 0.04f + static_cast<float>(QRandomGenerator::global()->generateDouble()) * 0.10f;
        starMesh->setRadius(baseRadius);

        const float x = -16.0f + static_cast<float>(QRandomGenerator::global()->generateDouble()) * 32.0f;
        const float y = -10.0f + static_cast<float>(QRandomGenerator::global()->generateDouble()) * 20.0f;
        const float z = -12.0f + static_cast<float>(QRandomGenerator::global()->generateDouble()) * 18.0f;
        tr->setTranslation(QVector3D(x, y, z));

        auto* mat = new Qt3DExtras::QPhongMaterial(star);
        const double hue = 200.0 + QRandomGenerator::global()->generateDouble() * 70.0;
        const double val = 0.65 + QRandomGenerator::global()->generateDouble() * 0.20;
        const QColor c = QColor::fromHsvF(hue / 360.0, 0.12, val);
        mat->setDiffuse(c);
        mat->setAmbient(QColor(8, 10, 14));
        mat->setSpecular(QColor(255, 255, 255));
        mat->setShininess(90.0f);

        auto* twinkle = new QPropertyAnimation(tr, "scale3D", this);
        twinkle->setStartValue(QVector3D(0.65f, 0.65f, 0.65f));
        twinkle->setEndValue(QVector3D(1.35f, 1.35f, 1.35f));
        twinkle->setDuration(QRandomGenerator::global()->bounded(1200, 3200));
        twinkle->setLoopCount(-1);
        twinkle->setEasingCurve(QEasingCurve::InOutSine);
        twinkle->start();

        star->addComponent(starMesh);
        star->addComponent(mat);
        star->addComponent(tr);
    }

    // 容器
    view3DContainer = QWidget::createWindowContainer(view3D, this);
    view3DContainer->setMinimumSize(0, 0);
    view3DContainer->show(); // Ensure it is shown
    view3DContainer->lower();
}

void PlayMenuWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (view3DContainer) {
        view3DContainer->setGeometry(rect());
        view3DContainer->lower();
    }
}

void PlayMenuWidget::multiModeButtonClicked() {
    if (gameWindow->getUserID() == "$#SINGLE#$") {
        AuthNoticeDialog* dialog = new AuthNoticeDialog("提示", "当前为离线模式无法进行多人游戏", 2, this);
        dialog->exec();
        return;
    }

    QTcpSocket socket;
    socket.connectToHost(QString::fromStdString(gameWindow->getIp()), std::stoi(gameWindow->getPort()));

    if (socket.waitForConnected(3000)) {
        socket.disconnectFromHost();
        NetDataIO* net = new NetDataIO(gameWindow->getIp(), gameWindow->getPort(), gameWindow);
        gameWindow->setNetDataIO(net);

        GameNetData joinMsg;
        joinMsg.setType(0);
        joinMsg.setID(gameWindow->getUserID());
        joinMsg.setData("ENTER");
        net->sendData(joinMsg);
    } else {
        AuthNoticeDialog* dialog = new AuthNoticeDialog("提示", "无法连接到服务器", 3, this);
        dialog->exec();
    }
}
