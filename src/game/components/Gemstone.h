#ifndef GEMSTONE_H
#define GEMSTONE_H

#include <string>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QGeometryRenderer>
#include <QPropertyAnimation>

class Gemstone : public Qt3DCore::QEntity {
    Q_OBJECT
public:
    Gemstone(int type, std::string style, Qt3DCore::QNode* parent = nullptr);
    ~Gemstone();

    int getType() const;
    void setType(int type);

    std::string getStyle() const;
    void setStyle(const std::string& style);

    Qt3DCore::QTransform* transform() const;

private:
    int type;
    std::string style;

    Qt3DCore::QTransform* m_transform;
    Qt3DExtras::QPhongMaterial* m_material;
    Qt3DRender::QGeometryRenderer* m_mesh;
    QPropertyAnimation* m_rotationAnimation;

    void updateAppearance();
    void setupMesh();
    void setupMaterial();
};

#endif // GEMSTONE_H
