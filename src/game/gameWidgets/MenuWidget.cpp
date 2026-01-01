#include "MenuWidget.h"
#include "../GameWindow.h"
#include "../components/MenuButton.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QResizeEvent>
#include <QApplication>
#include <QtMath>
#include <cmath>
#include <algorithm>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DExtras/qforwardrenderer.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>
#include <Qt3DRender/qcamera.h>
#include <Qt3DCore/qgeometry.h>
#include <Qt3DRender/qgeometryrenderer.h>
#include <Qt3DCore/qattribute.h>
#include <Qt3DCore/qbuffer.h>
#include <Qt3DExtras/QPerVertexColorMaterial>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QPointLight>
#include <QVector3D>
#include <QByteArray>
#include <QSurfaceFormat>
#include <QColor>
#include <QVector>
#include <QRandomGenerator>

MenuWidget::MenuWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    setupUI();
    setup3DView();
}

MenuWidget::~MenuWidget() {
    if (rootEntity) {
        delete rootEntity;
    }
    if (view3D) {
        delete view3D;
    }
}

void MenuWidget::setupUI() {
    setMinimumSize(1280, 720);
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(80, 40, 40, 40); // Increased left margin
    mainLayout->setSpacing(20);

    leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(24); // Increased spacing between buttons
    leftLayout->setAlignment(Qt::AlignVCenter); // Center buttons vertically

    // Button Configuration
    int btnW = 220;
    int btnH = 60;
    int fontSize = 20;

    playMenuButton = new MenuButton(btnW, btnH, fontSize, QColor(0, 255, 255), "开始游戏", this); // Cyan
    storeButton = new MenuButton(btnW, btnH, fontSize, QColor(0, 255, 0), "商店", this);         // Green
    achievementsButton = new MenuButton(btnW, btnH, fontSize, QColor(255, 255, 0), "成就", this); // Yellow
    ranklistButton = new MenuButton(btnW, btnH, fontSize, QColor(255, 165, 0), "排行榜", this);   // Orange
    settingsButton = new MenuButton(btnW, btnH, fontSize, QColor(255, 192, 203), "设置", this);   // Pink
    exitButton = new MenuButton(btnW, btnH, fontSize, QColor(255, 50, 50), "退出游戏", this);     // Red

    // Ensure buttons are on top of native 3D window
    playMenuButton->setAttribute(Qt::WA_NativeWindow);
    storeButton->setAttribute(Qt::WA_NativeWindow);
    achievementsButton->setAttribute(Qt::WA_NativeWindow);
    ranklistButton->setAttribute(Qt::WA_NativeWindow);
    settingsButton->setAttribute(Qt::WA_NativeWindow);
    exitButton->setAttribute(Qt::WA_NativeWindow);

    leftLayout->addWidget(playMenuButton);
    leftLayout->addWidget(storeButton);
    leftLayout->addWidget(achievementsButton);
    leftLayout->addWidget(ranklistButton);
    leftLayout->addWidget(settingsButton);
    leftLayout->addWidget(exitButton);
    
    // Add stretch to push buttons to center if needed, but Alignment handles it.
    // However, since mainLayout adds leftLayout, we need to ensure leftLayout takes space.

    connect(playMenuButton, &QPushButton::clicked, this, &MenuWidget::playMenuButtonClicked);
    connect(storeButton, &QPushButton::clicked, this, &MenuWidget::storeButtonClicked);
    connect(achievementsButton, &QPushButton::clicked, this, &MenuWidget::achievementsButtonClicked);
    connect(ranklistButton, &QPushButton::clicked, this, &MenuWidget::ranklistButtonClicked);
    connect(settingsButton, &QPushButton::clicked, this, &MenuWidget::settingsButtonClicked);
    connect(exitButton, &QPushButton::clicked, this, &MenuWidget::exitButtonClicked);

    mainLayout->addLayout(leftLayout, 0);
    mainLayout->addStretch(1); // Push everything else to the right
   
}

void MenuWidget::setup3DView() {
    view3D = new Qt3DExtras::Qt3DWindow();
    
    auto* frameGraph = view3D->defaultFrameGraph();
    frameGraph->setClearColor(QColor(10, 10, 25));
    rootEntity = new Qt3DCore::QEntity();
    view3D->setRootEntity(rootEntity);

    auto camera = view3D->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0.0f, 0.0f, 13.0f));
    camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    auto* bgAnim = new QVariantAnimation(this);
    bgAnim->setStartValue(0.0);
    bgAnim->setEndValue(1.0);
    bgAnim->setDuration(22000);
    bgAnim->setLoopCount(-1);
    connect(bgAnim, &QVariantAnimation::valueChanged, this, [frameGraph](const QVariant& v) {
        constexpr double TwoPi = 6.2831853071795864769;
        const double t = v.toDouble();
        const double hue = std::fmod(240.0 + 55.0 * std::sin(TwoPi * t * 0.7) + 18.0 * std::sin(TwoPi * t * 1.9), 360.0);
        const double sat = std::clamp(0.35 + 0.25 * std::sin(TwoPi * t * 0.5 + 1.2), 0.0, 1.0);
        const double val = std::clamp(0.05 + 0.10 * (0.5 + 0.5 * std::sin(TwoPi * t * 0.35)), 0.0, 1.0);
        frameGraph->setClearColor(QColor::fromHsvF(hue / 360.0, sat, val));
    });
    bgAnim->start();

    // Lights
    Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QDirectionalLight* light = new Qt3DRender::QDirectionalLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(0.38f);
    light->setWorldDirection(QVector3D(-1.0f, -1.0f, -1.0f));
    lightEntity->addComponent(light);

    Qt3DCore::QEntity* pointLightEntity1 = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* pointLight1 = new Qt3DRender::QPointLight(pointLightEntity1);
    pointLight1->setColor(QColor(255, 0, 255)); // Magenta
    pointLight1->setIntensity(1.4f);
    pointLightEntity1->addComponent(pointLight1);
    auto pl1Transform = new Qt3DCore::QTransform(pointLightEntity1);
    pl1Transform->setTranslation(QVector3D(5.0f, 5.0f, 5.0f));
    pointLightEntity1->addComponent(pl1Transform);

    Qt3DCore::QEntity* pointLightEntity2 = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* pointLight2 = new Qt3DRender::QPointLight(pointLightEntity2);
    pointLight2->setColor(QColor(0, 255, 255)); // Cyan
    pointLight2->setIntensity(1.4f);
    pointLightEntity2->addComponent(pointLight2);
    auto pl2Transform = new Qt3DCore::QTransform(pointLightEntity2);
    pl2Transform->setTranslation(QVector3D(-5.0f, -5.0f, 5.0f));
    pointLightEntity2->addComponent(pl2Transform);

    auto createOrbitingLight = [&](float radius, float y, float z, int durationMs, double hueStartDeg, float intensity) {
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
        pointLight->setIntensity(intensity);
        lightEntity->addComponent(pointLight);

        auto* lightTransform = new Qt3DCore::QTransform();
        lightTransform->setTranslation(QVector3D(radius, y, z));
        lightEntity->addComponent(lightTransform);

        auto* colorAnim = new QVariantAnimation(this);
        colorAnim->setStartValue(0.0);
        colorAnim->setEndValue(1.0);
        colorAnim->setDuration(std::max(4800, durationMs / 2));
        colorAnim->setLoopCount(-1);
        connect(colorAnim, &QVariantAnimation::valueChanged, this, [pointLight, hueStartDeg](const QVariant& v) {
            const double t = v.toDouble();
            const double hue = std::fmod(hueStartDeg + 360.0 * t, 360.0);
            pointLight->setColor(QColor::fromHsvF(hue / 360.0, 0.9, 1.0));
        });
        colorAnim->start();
    };

    createOrbitingLight(6.5f, 3.2f, 6.5f, 9000, 190.0, 1.6f);
    createOrbitingLight(6.5f, -2.8f, 6.5f, 13000, 300.0, 1.4f);

    // --- Octahedron (Central) ---
    // Geometry - Flat Shaded Octahedron (24 vertices for distinct face colors)
    auto geometry = new Qt3DCore::QGeometry(rootEntity);

    // Define the 6 raw vertices of an octahedron
    QVector3D top(0.0f, 1.0f, 0.0f);
    QVector3D bottom(0.0f, -1.0f, 0.0f);
    QVector3D front(0.0f, 0.0f, 1.0f);
    QVector3D back(0.0f, 0.0f, -1.0f);
    QVector3D left(-1.0f, 0.0f, 0.0f);
    QVector3D right(1.0f, 0.0f, 0.0f);

    struct Vertex {
        float x, y, z;
        float r, g, b;
    };
    QVector<Vertex> vertices;

    // Helper to add a face
    auto addFace = [&](const QVector3D& v1, const QVector3D& v2, const QVector3D& v3, const QVector3D& c) {
        vertices.append({v1.x(), v1.y(), v1.z(), c.x(), c.y(), c.z()});
        vertices.append({v2.x(), v2.y(), v2.z(), c.x(), c.y(), c.z()});
        vertices.append({v3.x(), v3.y(), v3.z(), c.x(), c.y(), c.z()});
    };

    // Define colors for the 8 faces
    QVector3D c1(1.0f, 0.0f, 0.0f); // Red
    QVector3D c2(0.0f, 1.0f, 0.0f); // Green
    QVector3D c3(0.0f, 0.0f, 1.0f); // Blue
    QVector3D c4(1.0f, 1.0f, 0.0f); // Yellow
    QVector3D c5(1.0f, 0.0f, 1.0f); // Magenta
    QVector3D c6(0.0f, 1.0f, 1.0f); // Cyan
    QVector3D c7(1.0f, 0.5f, 0.0f); // Orange
    QVector3D c8(0.5f, 0.0f, 1.0f); // Purple

    // Top Hemisphere
    addFace(top, front, right, c1);
    addFace(top, right, back, c2);
    addFace(top, back, left, c3);
    addFace(top, left, front, c4);

    // Bottom Hemisphere
    addFace(bottom, right, front, c5);
    addFace(bottom, back, right, c6);
    addFace(bottom, left, back, c7);
    addFace(bottom, front, left, c8);

    // Create Buffer
    QByteArray bufferBytes;
    bufferBytes.resize(vertices.size() * sizeof(Vertex));
    memcpy(bufferBytes.data(), vertices.data(), bufferBytes.size());

    auto buffer = new Qt3DCore::QBuffer(geometry);
    buffer->setData(bufferBytes);

    // Position Attribute
    auto posAttr = new Qt3DCore::QAttribute(geometry);
    posAttr->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    posAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    posAttr->setBuffer(buffer);
    posAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
    posAttr->setVertexSize(3);
    posAttr->setByteOffset(0);
    posAttr->setByteStride(sizeof(Vertex));
    posAttr->setCount(vertices.size());
    geometry->addAttribute(posAttr);

    // Color Attribute
    auto colAttr = new Qt3DCore::QAttribute(geometry);
    colAttr->setName(Qt3DCore::QAttribute::defaultColorAttributeName());
    colAttr->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    colAttr->setBuffer(buffer);
    colAttr->setVertexBaseType(Qt3DCore::QAttribute::Float);
    colAttr->setVertexSize(3);
    colAttr->setByteOffset(3 * sizeof(float));
    colAttr->setByteStride(sizeof(Vertex));
    colAttr->setCount(vertices.size());
    geometry->addAttribute(colAttr);

    // Renderer
    auto renderer = new Qt3DRender::QGeometryRenderer(rootEntity);
    renderer->setGeometry(geometry);
    renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

    // Entity & Material
    octahedronEntity = new Qt3DCore::QEntity(rootEntity);
    auto material = new Qt3DExtras::QPerVertexColorMaterial(octahedronEntity);
    
    octahedronTransform = new Qt3DCore::QTransform(octahedronEntity);
    octahedronTransform->setScale3D(QVector3D(2.5f, 2.5f, 2.5f));
    octahedronTransform->setRotationX(30.0f);

    octahedronEntity->addComponent(renderer);
    octahedronEntity->addComponent(material);
    octahedronEntity->addComponent(octahedronTransform);

    rotationAnim = new QPropertyAnimation(octahedronTransform, "rotationY", this);
    rotationAnim->setStartValue(0.0f);
    rotationAnim->setEndValue(360.0f);
    rotationAnim->setDuration(5000);
    rotationAnim->setLoopCount(-1);
    rotationAnim->start();

    // --- Orbiting Torus (Smaller & Centered) ---
    auto torusEntity = new Qt3DCore::QEntity(rootEntity);
    auto torusMesh = new Qt3DExtras::QTorusMesh(rootEntity);
    torusMesh->setRadius(3.5f);
    torusMesh->setMinorRadius(0.15f);
    torusMesh->setRings(64);
    torusMesh->setSlices(32);

    auto torusMaterial = new Qt3DExtras::QPhongMaterial(rootEntity);
    torusMaterial->setDiffuse(QColor(75, 150, 200));
    torusMaterial->setShininess(120.0f);
    torusMaterial->setAmbient(QColor(0, 50, 100));

    auto torusTransform = new Qt3DCore::QTransform(torusEntity);
    torusTransform->setRotationX(60.0f);

    torusEntity->addComponent(torusMesh);
    torusEntity->addComponent(torusMaterial);
    torusEntity->addComponent(torusTransform);

    // Animate Torus
    auto torusAnim = new QPropertyAnimation(torusTransform, "rotationY", this);
    torusAnim->setStartValue(0.0f);
    torusAnim->setEndValue(360.0f);
    torusAnim->setDuration(12000);
    torusAnim->setLoopCount(-1);
    torusAnim->start();

    auto* haloEntity = new Qt3DCore::QEntity(rootEntity);
    auto* haloMesh = new Qt3DExtras::QTorusMesh(haloEntity);
    haloMesh->setRadius(4.8f);
    haloMesh->setMinorRadius(0.07f);
    haloMesh->setRings(96);
    haloMesh->setSlices(48);
    auto* haloMat = new Qt3DExtras::QPhongMaterial(haloEntity);
    haloMat->setDiffuse(QColor(80, 135, 195));
    haloMat->setAmbient(QColor(10, 20, 35));
    haloMat->setSpecular(QColor(255, 255, 255));
    haloMat->setShininess(180.0f);
    auto* haloTr = new Qt3DCore::QTransform(haloEntity);
    haloTr->setRotationZ(30.0f);
    haloEntity->addComponent(haloMesh);
    haloEntity->addComponent(haloMat);
    haloEntity->addComponent(haloTr);
    auto* haloAnim = new QPropertyAnimation(haloTr, "rotationY", this);
    haloAnim->setStartValue(0.0f);
    haloAnim->setEndValue(-360.0f);
    haloAnim->setDuration(18000);
    haloAnim->setLoopCount(-1);
    haloAnim->start();

    // --- Orbiting Objects (Various Shapes) ---
    auto createOrbitingObject = [&](Qt3DRender::QGeometryRenderer* mesh, float radius, float speed, float yOffset, const QColor& color) {
        auto objEntity = new Qt3DCore::QEntity(rootEntity);
        
        auto objMat = new Qt3DExtras::QPhongMaterial(rootEntity);
        objMat->setDiffuse(color);
        objMat->setAmbient(color.darker());
        objMat->setSpecular(Qt::white);
        objMat->setShininess(150.0f);
        
        auto objTransform = new Qt3DCore::QTransform(objEntity);
        
        objEntity->addComponent(mesh);
        objEntity->addComponent(objMat);
        objEntity->addComponent(objTransform);
        
        // Pivot for orbit
        auto pivotEntity = new Qt3DCore::QEntity(rootEntity);
        auto pivotTransform = new Qt3DCore::QTransform(pivotEntity);
        pivotEntity->addComponent(pivotTransform);
        objEntity->setParent(pivotEntity);
        
        objTransform->setTranslation(QVector3D(radius, yOffset, 0.0f));
        objTransform->setScale3D(QVector3D(0.6f, 0.6f, 0.6f));
        
        // Self rotation
        auto selfRotAnim = new QPropertyAnimation(objTransform, "rotationY", this);
        selfRotAnim->setStartValue(0.0f);
        selfRotAnim->setEndValue(360.0f);
        selfRotAnim->setDuration(2000);
        selfRotAnim->setLoopCount(-1);
        selfRotAnim->start();

        // Pivot rotation (orbit)
        auto pivotAnim = new QPropertyAnimation(pivotTransform, "rotationY", this);
        pivotAnim->setStartValue(0.0f);
        pivotAnim->setEndValue(360.0f);
        pivotAnim->setDuration(speed);
        pivotAnim->setLoopCount(-1);
        pivotAnim->start();
    };

    // 1. Red Cube
    auto cubeMesh = new Qt3DExtras::QCuboidMesh(rootEntity);
    cubeMesh->setXExtent(1.0f); cubeMesh->setYExtent(1.0f); cubeMesh->setZExtent(1.0f);
    createOrbitingObject(cubeMesh, 5.0f, 5000, 1.5f, QColor(200, 70, 70));

    // 2. Green Sphere
    auto sphereMesh = new Qt3DExtras::QSphereMesh(rootEntity);
    sphereMesh->setRadius(0.6f);
    createOrbitingObject(sphereMesh, 6.5f, 7000, -1.5f, QColor(70, 200, 70));

    // 3. Blue Cylinder
    auto cylMesh = new Qt3DExtras::QCylinderMesh(rootEntity);
    cylMesh->setRadius(0.5f); cylMesh->setLength(1.2f);
    createOrbitingObject(cylMesh, 8.0f, 9000, 2.0f, QColor(65, 85, 205));

    // 4. Yellow Cone
    auto coneMesh = new Qt3DExtras::QConeMesh(rootEntity);
    coneMesh->setBottomRadius(0.6f); coneMesh->setLength(1.2f);
    createOrbitingObject(coneMesh, 5.5f, 4000, -2.5f, QColor(210, 210, 0));

    // --- Meteors (Shooting Stars) ---
    auto createMeteor = [&](const QVector3D& start, const QVector3D& end, int duration) {
        auto meteorEntity = new Qt3DCore::QEntity(rootEntity);
        auto meteorMesh = new Qt3DExtras::QSphereMesh(rootEntity);
        meteorMesh->setRadius(0.15f);
        
        auto meteorMat = new Qt3DExtras::QPhongMaterial(rootEntity);
        meteorMat->setDiffuse(QColor(210, 210, 210));
        meteorMat->setAmbient(QColor(210, 210, 210));
        meteorMat->setShininess(0.0f);
        
        auto meteorTransform = new Qt3DCore::QTransform(meteorEntity);
        meteorTransform->setTranslation(start);
        
        meteorEntity->addComponent(meteorMesh);
        meteorEntity->addComponent(meteorMat);
        meteorEntity->addComponent(meteorTransform);
        
        auto anim = new QPropertyAnimation(meteorTransform, "translation", this);
        anim->setStartValue(start);
        anim->setEndValue(end);
        anim->setDuration(duration);
        anim->setLoopCount(-1);
        anim->start();
    };

    createMeteor(QVector3D(-20, 8, -5), QVector3D(20, -8, -5), 2000);
    createMeteor(QVector3D(-25, 5, -10), QVector3D(25, -5, -10), 3500);
    createMeteor(QVector3D(20, 10, -8), QVector3D(-20, -10, -8), 2500);

    auto* starsRoot = new Qt3DCore::QEntity(rootEntity);
    auto* starMesh = new Qt3DExtras::QSphereMesh(starsRoot);
    starMesh->setRadius(0.06f);
    starMesh->setRings(10);
    starMesh->setSlices(10);

    const int starCount = 120;
    for (int i = 0; i < starCount; ++i) {
        auto* star = new Qt3DCore::QEntity(starsRoot);
        auto* tr = new Qt3DCore::QTransform();

        const float x = -14.0f + static_cast<float>(QRandomGenerator::global()->generateDouble()) * 28.0f;
        const float y = -10.0f + static_cast<float>(QRandomGenerator::global()->generateDouble()) * 20.0f;
        const float z = -14.0f + static_cast<float>(QRandomGenerator::global()->generateDouble()) * 18.0f;
        tr->setTranslation(QVector3D(x, y, z));
        const float s = 0.35f + static_cast<float>(QRandomGenerator::global()->generateDouble()) * 1.25f;
        tr->setScale3D(QVector3D(s, s, s));

        auto* mat = new Qt3DExtras::QPhongMaterial(star);
        const double hue = std::fmod(180.0 + QRandomGenerator::global()->generateDouble() * 180.0, 360.0);
        const QColor c = QColor::fromHsvF(hue / 360.0, 0.28, 0.82);
        mat->setDiffuse(c);
        mat->setAmbient(QColor(8, 8, 12));
        mat->setSpecular(QColor(255, 255, 255));
        mat->setShininess(70.0f);

        auto* twinkle = new QPropertyAnimation(tr, "scale3D", this);
        twinkle->setStartValue(QVector3D(0.65f * s, 0.65f * s, 0.65f * s));
        twinkle->setEndValue(QVector3D(1.35f * s, 1.35f * s, 1.35f * s));
        twinkle->setDuration(QRandomGenerator::global()->bounded(1400, 3600));
        twinkle->setLoopCount(-1);
        twinkle->setEasingCurve(QEasingCurve::InOutSine);
        twinkle->start();

        star->addComponent(starMesh);
        star->addComponent(mat);
        star->addComponent(tr);
    }

    // Container - Full Background
    view3DContainer = QWidget::createWindowContainer(view3D, this);
    view3DContainer->setMinimumSize(0, 0);
    view3DContainer->lower();
}

void MenuWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (view3DContainer) {
        view3DContainer->setGeometry(rect());
        view3DContainer->lower(); // Ensure it stays behind
    }
}

void MenuWidget::setBackgroundImage(const QPixmap& pixmap) {
    backgroundPixmap = pixmap;
    hasBackground = !backgroundPixmap.isNull();
    update();
}

void MenuWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    if (hasBackground) {
        // 背景图自适应拉伸，保持透明度
        QPixmap scaled = backgroundPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        p.setOpacity(0.8); // 背景图透明度，避免遮挡3D元素
        p.drawPixmap(0, 0, scaled);
        p.setOpacity(1.0);
    }
}

void MenuWidget::playMenuButtonClicked() {
    emit startGame();
}

void MenuWidget::storeButtonClicked() {
    emit openStore();
}

void MenuWidget::achievementsButtonClicked() {
    emit openAchievements();
}

void MenuWidget::ranklistButtonClicked() {
    emit openLeaderboard();
}

void MenuWidget::settingsButtonClicked() {
    emit openSettings();
}

void MenuWidget::exitButtonClicked() {
    QApplication::quit();
}
