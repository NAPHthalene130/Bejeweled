#include "Gemstone.h"
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <QColor>
#include <QVector3D>
#include <QQuaternion>
#include <QRandomGenerator>
#include <Qt3DRender/QPickingSettings>
#include <QString>

Gemstone::Gemstone(int type, std::string style, Qt3DCore::QNode* parent) 
    : Qt3DCore::QEntity(parent), type(type), style(style) {
    
    m_transform = new Qt3DCore::QTransform(this);
    addComponent(m_transform);

    m_material = new Qt3DExtras::QPhongMaterial(this);
    addComponent(m_material);

    m_mesh = nullptr; // 将在 updateAppearance 中创建

    updateAppearance();

    // 设置旋转动画
    m_rotationAnimation = new QPropertyAnimation(m_transform, "rotationY", this);
    m_rotationAnimation->setStartValue(0.0f);
    m_rotationAnimation->setEndValue(360.0f);
    m_rotationAnimation->setDuration(3000 + QRandomGenerator::global()->bounded(2000)); // 随机速度
    m_rotationAnimation->setLoopCount(-1);
    m_rotationAnimation->start();

    // 设置对象选择器 - 用于响应鼠标点击
    m_picker = new Qt3DRender::QObjectPicker(this);
    m_picker->setHoverEnabled(true);
    m_picker->setDragEnabled(false);
    m_picker->setEnabled(true);
    addComponent(m_picker);

    // 连接所有 picker 信号以便调试
    connect(m_picker, &Qt3DRender::QObjectPicker::pressed, this, [this](Qt3DRender::QPickEvent* event) {
        qDebug() << "[Gemstone] PRESSED - Type:" << this->type;
        emit pickEvent("pressed");
    });

    connect(m_picker, &Qt3DRender::QObjectPicker::clicked, this, [this](Qt3DRender::QPickEvent* event) {
        qDebug() << "[Gemstone] CLICKED - Type:" << this->type;
        emit pickEvent("clicked");
        emit clicked(this);
    });

    connect(m_picker, &Qt3DRender::QObjectPicker::entered, this, [this]() {
        qDebug() << "[Gemstone] ENTERED - Type:" << this->type;
        emit pickEvent("entered");
    });

    connect(m_picker, &Qt3DRender::QObjectPicker::exited, this, [this]() {
        qDebug() << "[Gemstone] EXITED - Type:" << this->type;
        emit pickEvent("exited");
    });

    qDebug() << "[Gemstone] Created gemstone type" << this->type << "with picker enabled:" << m_picker->isEnabled();
}

Gemstone::~Gemstone() {
    clearSpecialEffects();
}

int Gemstone::getType() const {
    return type;
}

void Gemstone::setType(int type) {
    if (this->type != type) {
        this->type = type;
        updateAppearance();
    }
}

std::string Gemstone::getStyle() const {
    return style;
}

void Gemstone::setStyle(const std::string& style) {
    if (this->style != style) {
        this->style = style;
        updateAppearance();
    }
}

Qt3DCore::QTransform* Gemstone::transform() const {
    return m_transform;
}

void Gemstone::updateAppearance() {
    setupMesh();
    setupMaterial();
}

void Gemstone::setupMesh() {
    if (m_mesh) {
        removeComponent(m_mesh);
        delete m_mesh;
        m_mesh = nullptr;
    }

    if (style == "XXXXX") {
         // 特定样式的占位符 - 可能是文本网格或特定形状
         Qt3DExtras::QCuboidMesh* mesh = new Qt3DExtras::QCuboidMesh(this);
         mesh->setXExtent(0.8f); mesh->setYExtent(0.8f); mesh->setZExtent(0.8f);
         m_mesh = mesh;
    } else {
        // 默认样式：8种几何形状
        switch (type % 8) {
            case 0: // 球体
            {
                Qt3DExtras::QSphereMesh* mesh = new Qt3DExtras::QSphereMesh(this);
                mesh->setRadius(0.45f);
                mesh->setRings(20); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 1: // 立方体
            {
                Qt3DExtras::QCuboidMesh* mesh = new Qt3DExtras::QCuboidMesh(this);
                mesh->setXExtent(0.8f); mesh->setYExtent(0.8f); mesh->setZExtent(0.8f);
                m_mesh = mesh;
                break;
            }
            case 2: // 圆锥体
            {
                Qt3DExtras::QConeMesh* mesh = new Qt3DExtras::QConeMesh(this);
                mesh->setBottomRadius(0.5f);
                mesh->setLength(1.0f);
                mesh->setRings(10); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 3: // 圆柱体
            {
                Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh(this);
                mesh->setRadius(0.45f);
                mesh->setLength(0.9f);
                mesh->setRings(10); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 4: // 圆环体
            {
                Qt3DExtras::QTorusMesh* mesh = new Qt3DExtras::QTorusMesh(this);
                mesh->setRadius(0.4f);
                mesh->setMinorRadius(0.15f);
                mesh->setRings(20); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 5: // 六棱柱
            {
                Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh(this);
                mesh->setRadius(0.5f);
                mesh->setLength(0.8f);
                mesh->setRings(2); mesh->setSlices(6); // 六边形
                m_mesh = mesh;
                break;
            }
            case 6: // 金字塔（四棱锥）
            {
                Qt3DExtras::QConeMesh* mesh = new Qt3DExtras::QConeMesh(this);
                mesh->setBottomRadius(0.5f);
                mesh->setLength(0.9f);
                mesh->setRings(2); mesh->setSlices(4); // 正方形底座
                m_mesh = mesh;
                break;
            }
            case 7: // 三棱柱
            {
                Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh(this);
                mesh->setRadius(0.5f);
                mesh->setLength(0.8f);
                mesh->setRings(2); mesh->setSlices(3); // 三角形
                m_mesh = mesh;
                break;
            }
            default:
            {
                Qt3DExtras::QSphereMesh* mesh = new Qt3DExtras::QSphereMesh(this);
                mesh->setRadius(0.4f);
                m_mesh = mesh;
                break;
            }
        }
    }

    if (m_mesh) {
        addComponent(m_mesh);
    }
}

void Gemstone::setupMaterial() {
    QColor color;
    // 定义8种类型的颜色
    switch (type % 8) {
        case 0: color = QColor(255, 50, 50); break;   // 红色
        case 1: color = QColor(50, 255, 50); break;   // 绿色
        case 2: color = QColor(50, 50, 255); break;   // 蓝色
        case 3: color = QColor(255, 255, 50); break;  // 黄色
        case 4: color = QColor(255, 50, 255); break;  // 品红色
        case 5: color = QColor(50, 255, 255); break;  // 青色
        case 6: color = QColor(255, 150, 50); break;  // 橙色
        case 7: color = QColor(200, 200, 200); break; // 灰色/白色
        default: color = Qt::white; break;
    }

    m_material->setDiffuse(color);
    m_material->setAmbient(color.darker(150));
    m_material->setSpecular(Qt::white);
    m_material->setShininess(50.0f);
}

void Gemstone::setSpecial(bool special) {
    if (this->special != special) {
        this->special = special;
        updateSpecialEffects();
    }
}

bool Gemstone::isSpecial() const {
    return special;
}

bool Gemstone::getCanBeChosen() const {
    return canBeChosen;
}

void Gemstone::setCanBeChosen(bool can) {
    canBeChosen = can;
    if (m_picker) {
        m_picker->setEnabled(can);
    }
}

void Gemstone::updateSpecialEffects() {
    clearSpecialEffects();

    if (!special) {
        return;
    }

    // 1. Halo Effect (Glowing ring/sphere)
    m_haloEntity = new Qt3DCore::QEntity(this);
    
    // Use a torus for a "halo" ring effect, or a larger sphere for a "glow"
    // Let's use a Torus for a visible halo ring
    Qt3DExtras::QTorusMesh* haloMesh = new Qt3DExtras::QTorusMesh();
    haloMesh->setRadius(0.7f);
    haloMesh->setMinorRadius(0.05f);
    haloMesh->setRings(30);
    haloMesh->setSlices(30);

    Qt3DExtras::QPhongMaterial* haloMat = new Qt3DExtras::QPhongMaterial();
    haloMat->setDiffuse(QColor(255, 255, 200));
    haloMat->setAmbient(QColor(255, 255, 200));
    haloMat->setSpecular(Qt::white);
    haloMat->setShininess(100.0f);
    
    Qt3DCore::QTransform* haloTransform = new Qt3DCore::QTransform();
    // Tilt the halo slightly
    haloTransform->setRotationX(30.0f);

    m_haloEntity->addComponent(haloMesh);
    m_haloEntity->addComponent(haloMat);
    m_haloEntity->addComponent(haloTransform);

    // Pulse animation for the halo
    m_haloScaleAnimation = new QPropertyAnimation(haloTransform, "scale3D");
    m_haloScaleAnimation->setStartValue(QVector3D(1.0f, 1.0f, 1.0f));
    m_haloScaleAnimation->setEndValue(QVector3D(1.2f, 1.2f, 1.2f));
    m_haloScaleAnimation->setDuration(1000);
    m_haloScaleAnimation->setLoopCount(-1);
    m_haloScaleAnimation->setEasingCurve(QEasingCurve::InOutSine);
    m_haloScaleAnimation->start();

    // 2. Particle Effects (Orbiting small spheres)
    m_particlesRoot = new Qt3DCore::QEntity(this);
    int particleCount = 6;
    
    for (int i = 0; i < particleCount; ++i) {
        Qt3DCore::QEntity* pEntity = new Qt3DCore::QEntity(m_particlesRoot);
        
        Qt3DExtras::QSphereMesh* pMesh = new Qt3DExtras::QSphereMesh();
        pMesh->setRadius(0.08f);
        
        Qt3DExtras::QPhongMaterial* pMat = new Qt3DExtras::QPhongMaterial();
        // Gold/Light color
        pMat->setDiffuse(QColor(255, 255, 100));
        pMat->setAmbient(QColor(255, 255, 0));
        pMat->setShininess(50.0f);
        
        Qt3DCore::QTransform* pTransform = new Qt3DCore::QTransform();
        
        pEntity->addComponent(pMesh);
        pEntity->addComponent(pMat);
        pEntity->addComponent(pTransform);
        
        m_particleEntities.push_back(pEntity);

        // Orbit animation
        // We animate the rotation of the particle around the center.
        // Simple way: calculate position based on angle and radius in a property animation?
        // Or cleaner: Parent the particle to a pivot, rotate the pivot.
        
        // Let's use pivot approach for simpler code, but wait, if I create a pivot for each particle...
        // Actually, let's just make them orbit using simple circular math if possible, 
        // but QPropertyAnimation works on properties.
        // Let's create a pivot entity for EACH particle at (0,0,0) and parent the particle to it, 
        // then offset the particle, then rotate the pivot.
        
        Qt3DCore::QEntity* pivot = new Qt3DCore::QEntity(m_particlesRoot);
        Qt3DCore::QTransform* pivotTransform = new Qt3DCore::QTransform();
        pivot->addComponent(pivotTransform);
        
        // Reparent particle to pivot
        pEntity->setParent(pivot);
        pTransform->setTranslation(QVector3D(0.9f, 0.0f, 0.0f)); // Radius 0.9
        
        // Rotate pivot
        QPropertyAnimation* orbitAnim = new QPropertyAnimation(pivotTransform, "rotationY");
        orbitAnim->setStartValue(0.0f);
        orbitAnim->setEndValue(360.0f);
        // Random speed and direction
        int duration = 1500 + QRandomGenerator::global()->bounded(1500);
        orbitAnim->setDuration(duration);
        orbitAnim->setLoopCount(-1);
        
        // Randomize start angle
        float startAngle = (360.0f / particleCount) * i;
        pivotTransform->setRotationY(startAngle);
        orbitAnim->setStartValue(startAngle);
        orbitAnim->setEndValue(startAngle + 360.0f);
        
        // Random axis tilt
        float tiltX = QRandomGenerator::global()->bounded(60) - 30;
        float tiltZ = QRandomGenerator::global()->bounded(60) - 30;
        QQuaternion tilt = QQuaternion::fromEulerAngles(tiltX, 0, tiltZ);
        // We can't easily tilt the pivot AND rotate Y local without complex transform hierarchy.
        // But for simplicity, let's just rotate Y.
        // If we want tilted orbits, we can put the pivot inside ANOTHER pivot that is tilted.
        
        Qt3DCore::QEntity* tiltRoot = new Qt3DCore::QEntity(m_particlesRoot);
        Qt3DCore::QTransform* tiltTransform = new Qt3DCore::QTransform();
        tiltTransform->setRotation(tilt);
        tiltRoot->addComponent(tiltTransform);
        
        pivot->setParent(tiltRoot);

        orbitAnim->start();
        m_particleAnimations.push_back(orbitAnim);
    }
}

void Gemstone::clearSpecialEffects() {
    if (m_haloEntity) {
        m_haloEntity->setParent((Qt3DCore::QNode*)nullptr);
        delete m_haloEntity;
        m_haloEntity = nullptr;
    }
    if (m_haloScaleAnimation) {
        // Animation is child of entity or we manage it? 
        // We created it with parent as transform or similar? 
        // Usually it's better to explicitly delete if we stored pointer and didn't parent it to QObject that gets deleted.
        // But m_haloScaleAnimation parent is likely the target object or we didn't set parent.
        // Let's safe delete.
        delete m_haloScaleAnimation;
        m_haloScaleAnimation = nullptr;
    }

    if (m_particlesRoot) {
        m_particlesRoot->setParent((Qt3DCore::QNode*)nullptr);
        delete m_particlesRoot;
        m_particlesRoot = nullptr;
    }
    
    m_particleEntities.clear();
    // Animations are likely deleted when their parents (transforms/entities) are deleted if properly parented, 
    // but here we created them with 'new QPropertyAnimation(target, prop)'. 
    // If target is deleted, animation might dangle if not parented to a QObject.
    // It is safer to delete them.
    for (auto anim : m_particleAnimations) {
        delete anim;
    }
    m_particleAnimations.clear();
}