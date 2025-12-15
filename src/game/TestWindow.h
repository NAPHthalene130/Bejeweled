#ifndef TEST_WINDOW_H
#define TEST_WINDOW_H

#include <QWidget>
#include <vector>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>

class Gemstone;

class TestWindow : public QWidget {
    Q_OBJECT
public:
    explicit TestWindow(QWidget* parent = nullptr);
    ~TestWindow();

private:
    std::vector<Gemstone*> gemstones;
    
    // 3D Window components
    Qt3DExtras::Qt3DWindow* view3D;
    QWidget* container;
    Qt3DCore::QEntity* rootEntity;
    Qt3DRender::QCamera* cameraEntity;
    Qt3DCore::QEntity* lightEntity;

    void setup3DScene();
};

#endif // TEST_WINDOW_H
