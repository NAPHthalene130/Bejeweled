#include "MenuWidget.h"
#include "../GameWindow.h"
#include "../components/MenuButton.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QApplication>
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

MenuWidget::MenuWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    setupUI();
    setup3DView();
}

void MenuWidget::setupUI() {
    setMinimumSize(1600, 1000);
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
#ifdef HAVE_QT3D
    view3D = new Qt3DExtras::Qt3DWindow();
    
    // Dark Space Background
    view3D->defaultFrameGraph()->setClearColor(QColor(10, 10, 25)); // Darker
    rootEntity = new Qt3DCore::QEntity();
    view3D->setRootEntity(rootEntity);

    auto camera = view3D->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0.0f, 0.0f, 13.0f));
    camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));

    // Lights
    Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QDirectionalLight* light = new Qt3DRender::QDirectionalLight(lightEntity);
    light->setColor(Qt::white);
    light->setIntensity(0.5f);
    light->setWorldDirection(QVector3D(-1.0f, -1.0f, -1.0f));
    lightEntity->addComponent(light);

    Qt3DCore::QEntity* pointLightEntity1 = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* pointLight1 = new Qt3DRender::QPointLight(pointLightEntity1);
    pointLight1->setColor(QColor(255, 0, 255)); // Magenta
    pointLight1->setIntensity(2.0f);
    pointLightEntity1->addComponent(pointLight1);
    auto pl1Transform = new Qt3DCore::QTransform();
    pl1Transform->setTranslation(QVector3D(5.0f, 5.0f, 5.0f));
    pointLightEntity1->addComponent(pl1Transform);

    Qt3DCore::QEntity* pointLightEntity2 = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* pointLight2 = new Qt3DRender::QPointLight(pointLightEntity2);
    pointLight2->setColor(QColor(0, 255, 255)); // Cyan
    pointLight2->setIntensity(2.0f);
    pointLightEntity2->addComponent(pointLight2);
    auto pl2Transform = new Qt3DCore::QTransform();
    pl2Transform->setTranslation(QVector3D(-5.0f, -5.0f, 5.0f));
    pointLightEntity2->addComponent(pl2Transform);

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
    
    octahedronTransform = new Qt3DCore::QTransform();
    octahedronTransform->setScale3D(QVector3D(2.5f, 2.5f, 2.5f));
    octahedronTransform->setRotationX(30.0f);

    octahedronEntity->addComponent(renderer);
    octahedronEntity->addComponent(material);
    octahedronEntity->addComponent(octahedronTransform);

    rotationAnim = new QPropertyAnimation(octahedronTransform, "rotationY");
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
    torusMaterial->setDiffuse(QColor(100, 200, 255));
    torusMaterial->setShininess(150.0f);
    torusMaterial->setAmbient(QColor(0, 50, 100));

    auto torusTransform = new Qt3DCore::QTransform();
    torusTransform->setRotationX(60.0f);

    torusEntity->addComponent(torusMesh);
    torusEntity->addComponent(torusMaterial);
    torusEntity->addComponent(torusTransform);

    // Animate Torus
    auto torusAnim = new QPropertyAnimation(torusTransform, "rotationY");
    torusAnim->setStartValue(0.0f);
    torusAnim->setEndValue(360.0f);
    torusAnim->setDuration(12000);
    torusAnim->setLoopCount(-1);
    torusAnim->start();

    // --- Orbiting Objects (Various Shapes) ---
    auto createOrbitingObject = [&](Qt3DRender::QGeometryRenderer* mesh, float radius, float speed, float yOffset, const QColor& color) {
        auto objEntity = new Qt3DCore::QEntity(rootEntity);
        
        auto objMat = new Qt3DExtras::QPhongMaterial(rootEntity);
        objMat->setDiffuse(color);
        objMat->setAmbient(color.darker());
        objMat->setSpecular(Qt::white);
        objMat->setShininess(50.0f);
        
        auto objTransform = new Qt3DCore::QTransform();
        
        objEntity->addComponent(mesh);
        objEntity->addComponent(objMat);
        objEntity->addComponent(objTransform);
        
        // Pivot for orbit
        auto pivotEntity = new Qt3DCore::QEntity(rootEntity);
        auto pivotTransform = new Qt3DCore::QTransform();
        pivotEntity->addComponent(pivotTransform);
        objEntity->setParent(pivotEntity);
        
        objTransform->setTranslation(QVector3D(radius, yOffset, 0.0f));
        
        // Self rotation
        auto selfRotAnim = new QPropertyAnimation(objTransform, "rotationY");
        selfRotAnim->setStartValue(0.0f);
        selfRotAnim->setEndValue(360.0f);
        selfRotAnim->setDuration(2000);
        selfRotAnim->setLoopCount(-1);
        selfRotAnim->start();

        // Pivot rotation (orbit)
        auto pivotAnim = new QPropertyAnimation(pivotTransform, "rotationY");
        pivotAnim->setStartValue(0.0f);
        pivotAnim->setEndValue(360.0f);
        pivotAnim->setDuration(speed);
        pivotAnim->setLoopCount(-1);
        pivotAnim->start();
    };

    // 1. Red Cube
    auto cubeMesh = new Qt3DExtras::QCuboidMesh(rootEntity);
    cubeMesh->setXExtent(1.0f); cubeMesh->setYExtent(1.0f); cubeMesh->setZExtent(1.0f);
    createOrbitingObject(cubeMesh, 5.0f, 5000, 1.5f, QColor(255, 80, 80));

    // 2. Green Sphere
    auto sphereMesh = new Qt3DExtras::QSphereMesh(rootEntity);
    sphereMesh->setRadius(0.6f);
    createOrbitingObject(sphereMesh, 6.5f, 7000, -1.5f, QColor(80, 255, 80));

    // 3. Blue Cylinder
    auto cylMesh = new Qt3DExtras::QCylinderMesh(rootEntity);
    cylMesh->setRadius(0.5f); cylMesh->setLength(1.2f);
    createOrbitingObject(cylMesh, 8.0f, 9000, 2.0f, QColor(80, 100, 255));

    // 4. Yellow Cone
    auto coneMesh = new Qt3DExtras::QConeMesh(rootEntity);
    coneMesh->setBottomRadius(0.6f); coneMesh->setLength(1.2f);
    createOrbitingObject(coneMesh, 5.5f, 4000, -2.5f, QColor(255, 255, 0));

    // --- Meteors (Shooting Stars) ---
    auto createMeteor = [&](const QVector3D& start, const QVector3D& end, int duration) {
        auto meteorEntity = new Qt3DCore::QEntity(rootEntity);
        auto meteorMesh = new Qt3DExtras::QSphereMesh(rootEntity);
        meteorMesh->setRadius(0.15f);
        
        auto meteorMat = new Qt3DExtras::QPhongMaterial(rootEntity);
        meteorMat->setDiffuse(Qt::white);
        meteorMat->setAmbient(Qt::white);
        meteorMat->setShininess(0.0f);
        
        auto meteorTransform = new Qt3DCore::QTransform();
        meteorTransform->setTranslation(start);
        
        meteorEntity->addComponent(meteorMesh);
        meteorEntity->addComponent(meteorMat);
        meteorEntity->addComponent(meteorTransform);
        
        auto anim = new QPropertyAnimation(meteorTransform, "translation");
        anim->setStartValue(start);
        anim->setEndValue(end);
        anim->setDuration(duration);
        anim->setLoopCount(-1);
        anim->start();
    };

    createMeteor(QVector3D(-20, 8, -5), QVector3D(20, -8, -5), 2000);
    createMeteor(QVector3D(-25, 5, -10), QVector3D(25, -5, -10), 3500);
    createMeteor(QVector3D(20, 10, -8), QVector3D(-20, -10, -8), 2500);

    // Container - Full Background
    view3DContainer = QWidget::createWindowContainer(view3D, this);
    view3DContainer->setMinimumSize(0, 0);
    view3DContainer->lower();
#else
    view3D = nullptr;
    rootEntity = nullptr;
    octahedronEntity = nullptr;
    octahedronTransform = nullptr;
    rotationAnim = nullptr;
    view3DContainer = new QWidget(this);
    view3DContainer->setStyleSheet("background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(10,10,25,255), stop:1 rgba(30,30,60,255));");
    view3DContainer->setMinimumSize(0, 0);
    view3DContainer->lower();
#endif
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
    if (hasBackground) {
        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        QPixmap scaled = backgroundPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        p.drawPixmap(0, 0, scaled);
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
