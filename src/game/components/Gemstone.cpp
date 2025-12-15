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

    m_mesh = nullptr; // Will be created in updateAppearance

    updateAppearance();

    // Setup Rotation Animation
    m_rotationAnimation = new QPropertyAnimation(m_transform, "rotationY");
    m_rotationAnimation->setStartValue(0.0f);
    m_rotationAnimation->setEndValue(360.0f);
    m_rotationAnimation->setDuration(3000 + QRandomGenerator::global()->bounded(2000)); // Random speed
    m_rotationAnimation->setLoopCount(-1);
    m_rotationAnimation->start();
}

Gemstone::~Gemstone() {
    // Qt3D nodes are automatically cleaned up when parent is destroyed
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
         // Placeholder for specific style - maybe a text mesh or specific shape
         Qt3DExtras::QCuboidMesh* mesh = new Qt3DExtras::QCuboidMesh();
         mesh->setXExtent(0.8f); mesh->setYExtent(0.8f); mesh->setZExtent(0.8f);
         m_mesh = mesh;
    } else {
        // Default style: 8 types of Geometric shapes
        switch (type % 8) {
            case 0: // Sphere
            {
                Qt3DExtras::QSphereMesh* mesh = new Qt3DExtras::QSphereMesh();
                mesh->setRadius(0.45f);
                mesh->setRings(20); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 1: // Cube
            {
                Qt3DExtras::QCuboidMesh* mesh = new Qt3DExtras::QCuboidMesh();
                mesh->setXExtent(0.8f); mesh->setYExtent(0.8f); mesh->setZExtent(0.8f);
                m_mesh = mesh;
                break;
            }
            case 2: // Cone
            {
                Qt3DExtras::QConeMesh* mesh = new Qt3DExtras::QConeMesh();
                mesh->setBottomRadius(0.5f);
                mesh->setLength(1.0f);
                mesh->setRings(10); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 3: // Cylinder
            {
                Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh();
                mesh->setRadius(0.45f);
                mesh->setLength(0.9f);
                mesh->setRings(10); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 4: // Torus
            {
                Qt3DExtras::QTorusMesh* mesh = new Qt3DExtras::QTorusMesh();
                mesh->setRadius(0.4f);
                mesh->setMinorRadius(0.15f);
                mesh->setRings(20); mesh->setSlices(20);
                m_mesh = mesh;
                break;
            }
            case 5: // Hexagonal Prism
            {
                Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh();
                mesh->setRadius(0.5f);
                mesh->setLength(0.8f);
                mesh->setRings(2); mesh->setSlices(6); // Hexagon
                m_mesh = mesh;
                break;
            }
            case 6: // Pyramid (4-sided cone)
            {
                Qt3DExtras::QConeMesh* mesh = new Qt3DExtras::QConeMesh();
                mesh->setBottomRadius(0.5f);
                mesh->setLength(0.9f);
                mesh->setRings(2); mesh->setSlices(4); // Square base
                m_mesh = mesh;
                break;
            }
            case 7: // Triangular Prism
            {
                Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh();
                mesh->setRadius(0.5f);
                mesh->setLength(0.8f);
                mesh->setRings(2); mesh->setSlices(3); // Triangle
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
    // Define colors for 8 types
    switch (type % 8) {
        case 0: color = QColor(255, 50, 50); break;   // Red
        case 1: color = QColor(50, 255, 50); break;   // Green
        case 2: color = QColor(50, 50, 255); break;   // Blue
        case 3: color = QColor(255, 255, 50); break;  // Yellow
        case 4: color = QColor(255, 50, 255); break;  // Magenta
        case 5: color = QColor(50, 255, 255); break;  // Cyan
        case 6: color = QColor(255, 150, 50); break;  // Orange
        case 7: color = QColor(200, 200, 200); break; // Grey/White
        default: color = Qt::white; break;
    }

    m_material->setDiffuse(color);
    m_material->setAmbient(color.darker(150));
    m_material->setSpecular(Qt::white);
    m_material->setShininess(50.0f);
}
