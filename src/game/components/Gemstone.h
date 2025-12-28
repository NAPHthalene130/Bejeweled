#ifndef GEMSTONE_H
#define GEMSTONE_H

#include <string>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QObjectPicker>
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
    bool isSpecial() const;
    void setSpecial(bool special);

    bool getCanBeChosen() const;
    void setCanBeChosen(bool can);

signals:
    void clicked(Gemstone* self);
    void pickEvent(const QString& info);

private:
    int type;
    std::string style;
    bool special = false;
    bool canBeChosen = true;

    Qt3DCore::QTransform* m_transform;
    Qt3DExtras::QPhongMaterial* m_material;
    Qt3DRender::QGeometryRenderer* m_mesh;
    Qt3DRender::QObjectPicker* m_picker;
    QPropertyAnimation* m_rotationAnimation;

    // Special effects components
    Qt3DCore::QEntity* m_haloEntity = nullptr;
    QPropertyAnimation* m_haloScaleAnimation = nullptr;
    
    Qt3DCore::QEntity* m_particlesRoot = nullptr;
    std::vector<Qt3DCore::QEntity*> m_particleEntities;
    std::vector<QPropertyAnimation*> m_particleAnimations;

    void updateAppearance();
    void setupMesh();
    void setupMaterial();
    void updateSpecialEffects();
    void clearSpecialEffects();
};

#endif // GEMSTONE_H
