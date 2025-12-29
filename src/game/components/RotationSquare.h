#ifndef ROTATION_SQUARE_H
#define ROTATION_SQUARE_H

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QCuboidMesh>
#include <QVector3D>

// 2x2旋转框的可视化指示器
class RotationSquare : public Qt3DCore::QEntity {
    Q_OBJECT
public:
    explicit RotationSquare(Qt3DCore::QNode* parent = nullptr);
    ~RotationSquare();

    // 设置框的中心位置（左上角宝石和右下角宝石的中心）
    void setPosition(const QVector3D& topLeftPos, const QVector3D& bottomRightPos);
    void setVisible(bool visible);
    void setColor(const QColor& color);

private:
    Qt3DCore::QEntity* topEdge;
    Qt3DCore::QEntity* bottomEdge;
    Qt3DCore::QEntity* leftEdge;
    Qt3DCore::QEntity* rightEdge;

    Qt3DCore::QTransform* topTransform;
    Qt3DCore::QTransform* bottomTransform;
    Qt3DCore::QTransform* leftTransform;
    Qt3DCore::QTransform* rightTransform;

    Qt3DExtras::QPhongMaterial* material;
};

#endif // ROTATION_SQUARE_H
