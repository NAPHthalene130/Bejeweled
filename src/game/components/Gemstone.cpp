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

Gemstone::Gemstone(int type, std::string style, Qt3DCore::QNode* parent) 
    : Qt3DCore::QEntity(parent), type(type), style(style) {
    
    m_transform = new Qt3DCore::QTransform();
    addComponent(m_transform);

    m_material = new Qt3DExtras::QPhongMaterial();
    addComponent(m_material);

    m_mesh = nullptr; // 将在 updateAppearance 中创建

    updateAppearance();

    // 设置旋转动画
    m_rotationAnimation = new QPropertyAnimation(m_transform, "rotationY");
    m_rotationAnimation->setStartValue(0.0f);
    m_rotationAnimation->setEndValue(360.0f);
    m_rotationAnimation->setDuration(3000 + QRandomGenerator::global()->bounded(2000)); // 随机速度
    m_rotationAnimation->setLoopCount(-1);
    m_rotationAnimation->start();
}

Gemstone::~Gemstone() {
    // 当父节点被销毁时，Qt3D 节点会自动清理
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
         Qt3DExtras::QCuboidMesh* mesh = new Qt3DExtras::QCuboidMesh();
         mesh->setXExtent(0.8f); mesh->setYExtent(0.8f); mesh->setZExtent(0.8f);
         m_mesh = mesh;
    } else {
        // 默认样式：8种几何形状
        switch (type % 8) {
            case 0: // 球体
            {
                Qt3DExtras::QSphereMesh* mesh = new Qt3DExtras::QSphereMesh();
                mesh->setRadius(0.45f);
                mesh->setRings(20); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 1: // 立方体
            {
                Qt3DExtras::QCuboidMesh* mesh = new Qt3DExtras::QCuboidMesh();
                mesh->setXExtent(0.8f); mesh->setYExtent(0.8f); mesh->setZExtent(0.8f);
                m_mesh = mesh;
                break;
            }
            case 2: // 圆锥体
            {
                Qt3DExtras::QConeMesh* mesh = new Qt3DExtras::QConeMesh();
                mesh->setBottomRadius(0.5f);
                mesh->setLength(1.0f);
                mesh->setRings(10); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 3: // 圆柱体
            {
                Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh();
                mesh->setRadius(0.45f);
                mesh->setLength(0.9f);
                mesh->setRings(10); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 4: // 圆环体
            {
                Qt3DExtras::QTorusMesh* mesh = new Qt3DExtras::QTorusMesh();
                mesh->setRadius(0.4f);
                mesh->setMinorRadius(0.15f);
                mesh->setRings(20); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 5: // 六棱柱
            {
                Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh();
                mesh->setRadius(0.5f);
                mesh->setLength(0.8f);
                mesh->setRings(2); mesh->setSlices(6); // 六边形
                m_mesh = mesh;
                break;
            }
            case 6: // 金字塔（四棱锥）
            {
                Qt3DExtras::QConeMesh* mesh = new Qt3DExtras::QConeMesh();
                mesh->setBottomRadius(0.5f);
                mesh->setLength(0.9f);
                mesh->setRings(2); mesh->setSlices(4); // 正方形底座
                m_mesh = mesh;
                break;
            }
            case 7: // 三棱柱
            {
                Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh();
                mesh->setRadius(0.5f);
                mesh->setLength(0.8f);
                mesh->setRings(2); mesh->setSlices(3); // 三角形
                m_mesh = mesh;
                break;
            }
            default:
            {
                Qt3DExtras::QSphereMesh* mesh = new Qt3DExtras::QSphereMesh();
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
