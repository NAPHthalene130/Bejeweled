#ifndef SELECTED_CIRCLE_H
#define SELECTED_CIRCLE_H

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QTorusMesh>

class SelectedCircle : public Qt3DCore::QEntity {
    Q_OBJECT
public:
    explicit SelectedCircle(Qt3DCore::QNode* parent = nullptr);
    ~SelectedCircle();

    void setPosition(const QVector3D& pos);
    void setVisible(bool visible);
    void setColor(const QColor& color);

private:
    Qt3DCore::QTransform* m_transform;
    Qt3DExtras::QPhongMaterial* m_material;
    Qt3DExtras::QTorusMesh* m_mesh;
};

#endif // SELECTED_CIRCLE_H
