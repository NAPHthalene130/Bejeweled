#include "SelectedCircle.h"
#include <QColor>

SelectedCircle::SelectedCircle(Qt3DCore::QNode* parent) 
    : Qt3DCore::QEntity(parent) {
    
    // Transform
    m_transform = new Qt3DCore::QTransform();
    // Rotate 90 degrees around X to face the camera (assuming camera looks down Z)
    m_transform->setRotationX(90.0f);
    addComponent(m_transform);

    // Mesh (Torus)
    m_mesh = new Qt3DExtras::QTorusMesh();
    // Diameter 120 pixels maps to approx 1.5 units in 3D world (spacing is 1.5)
    // We want it slightly smaller than the full cell. 
    // Radius 0.6 -> Diameter 1.2
    m_mesh->setRadius(0.6f); 
    m_mesh->setMinorRadius(0.08f); // Thickness
    m_mesh->setRings(30);
    m_mesh->setSlices(30);
    addComponent(m_mesh);

    // Material
    m_material = new Qt3DExtras::QPhongMaterial();
    m_material->setDiffuse(QColor(255, 215, 0)); // Gold
    m_material->setAmbient(QColor(255, 215, 0));
    m_material->setSpecular(Qt::white);
    m_material->setShininess(150.0f);
    addComponent(m_material);

    setEnabled(false); // Default hidden
}

SelectedCircle::~SelectedCircle() {
}

void SelectedCircle::setPosition(const QVector3D& pos) {
    // Keep the rotation, update translation
    // We want the circle to be visible. 
    // Moving it slightly in FRONT of the gem (positive Z) to ensure visibility.
    // Gem is at Z=0. Camera is at Z=20.
    // So Z=1.0f puts it between gem and camera.
    m_transform->setTranslation(QVector3D(pos.x(), pos.y(), 1.0f));
}

void SelectedCircle::setVisible(bool visible) {
    setEnabled(visible);
}

void SelectedCircle::setColor(const QColor& color) {
    m_material->setDiffuse(color);
    m_material->setAmbient(color);
}
