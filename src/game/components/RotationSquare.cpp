#include "RotationSquare.h"
#include <Qt3DExtras/QCuboidMesh>

RotationSquare::RotationSquare(Qt3DCore::QNode* parent)
    : Qt3DCore::QEntity(parent) {

    // 创建材质（共用一个材质）
    material = new Qt3DExtras::QPhongMaterial(this);
    material->setDiffuse(QColor(255, 200, 0, 200)); // 金黄色
    material->setAmbient(QColor(255, 220, 0));
    material->setShininess(50.0f);

    // 创建四条边（使用细长的立方体）
    // 上边
    topEdge = new Qt3DCore::QEntity(this);
    auto* topMesh = new Qt3DExtras::QCuboidMesh();
    topMesh->setXExtent(1.5f);  // 宽度
    topMesh->setYExtent(0.05f);  // 厚度
    topMesh->setZExtent(0.1f);   // 深度
    topTransform = new Qt3DCore::QTransform();
    topEdge->addComponent(topMesh);
    topEdge->addComponent(topTransform);
    topEdge->addComponent(material);

    // 下边
    bottomEdge = new Qt3DCore::QEntity(this);
    auto* bottomMesh = new Qt3DExtras::QCuboidMesh();
    bottomMesh->setXExtent(1.5f);
    bottomMesh->setYExtent(0.05f);
    bottomMesh->setZExtent(0.1f);
    bottomTransform = new Qt3DCore::QTransform();
    bottomEdge->addComponent(bottomMesh);
    bottomEdge->addComponent(bottomTransform);
    bottomEdge->addComponent(material);

    // 左边
    leftEdge = new Qt3DCore::QEntity(this);
    auto* leftMesh = new Qt3DExtras::QCuboidMesh();
    leftMesh->setXExtent(0.05f);
    leftMesh->setYExtent(1.5f);
    leftMesh->setZExtent(0.1f);
    leftTransform = new Qt3DCore::QTransform();
    leftEdge->addComponent(leftMesh);
    leftEdge->addComponent(leftTransform);
    leftEdge->addComponent(material);

    // 右边
    rightEdge = new Qt3DCore::QEntity(this);
    auto* rightMesh = new Qt3DExtras::QCuboidMesh();
    rightMesh->setXExtent(0.05f);
    rightMesh->setYExtent(1.5f);
    rightMesh->setZExtent(0.1f);
    rightTransform = new Qt3DCore::QTransform();
    rightEdge->addComponent(rightMesh);
    rightEdge->addComponent(rightTransform);
    rightEdge->addComponent(material);

    setVisible(false);
}

RotationSquare::~RotationSquare() {
}

void RotationSquare::setPosition(const QVector3D& topLeftPos, const QVector3D& bottomRightPos) {
    // 计算中心点和尺寸
    float centerX = (topLeftPos.x() + bottomRightPos.x()) / 2.0f;
    float centerY = (topLeftPos.y() + bottomRightPos.y()) / 2.0f;
    float width = bottomRightPos.x() - topLeftPos.x();
    float height = topLeftPos.y() - bottomRightPos.y();

    // 稍微往前一点，避免被宝石遮挡
    float z = 0.3f;

    // 上边：在顶部中心
    topTransform->setTranslation(QVector3D(centerX, topLeftPos.y(), z));

    // 下边：在底部中心
    bottomTransform->setTranslation(QVector3D(centerX, bottomRightPos.y(), z));

    // 左边：在左侧中心
    leftTransform->setTranslation(QVector3D(topLeftPos.x(), centerY, z));

    // 右边：在右侧中心
    rightTransform->setTranslation(QVector3D(bottomRightPos.x(), centerY, z));
}

void RotationSquare::setVisible(bool visible) {
    topEdge->setEnabled(visible);
    bottomEdge->setEnabled(visible);
    leftEdge->setEnabled(visible);
    rightEdge->setEnabled(visible);
}

void RotationSquare::setColor(const QColor& color) {
    material->setDiffuse(color);
}
