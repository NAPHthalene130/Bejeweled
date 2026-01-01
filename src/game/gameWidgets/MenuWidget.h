#ifndef MENU_WIDGET_H
#define MENU_WIDGET_H
#include <QWidget>
#include <QPixmap>
#include <QColor>

class GameWindow;
class QVBoxLayout;
class QHBoxLayout;
class QPaintEvent;
class QPropertyAnimation;

namespace Qt3DExtras {
class Qt3DWindow;
class QPhongAlphaMaterial;
}
namespace Qt3DCore {
class QEntity;
class QTransform;
}
namespace Qt3DRender {
class QGeometry;
class QGeometryRenderer;
class QAttribute;
class QBuffer;
class QDirectionalLight;
class QPointLight;
}

class MenuButton;

class MenuWidget : public QWidget {
    Q_OBJECT
public:
    explicit MenuWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    ~MenuWidget();
    void setBackgroundImage(const QPixmap& pixmap);
signals:
    void startGame();
    void openStore();
    void openAchievements();
    void openLeaderboard();
    void openSettings();
    void openAbout();
private slots:
    void playMenuButtonClicked();
    void storeButtonClicked();
    void achievementsButtonClicked();
    void ranklistButtonClicked();
    void settingsButtonClicked();
    void aboutButtonClicked();
    void exitButtonClicked();
private:
    GameWindow* gameWindow = nullptr;
    MenuButton* playMenuButton = nullptr;
    MenuButton* storeButton = nullptr;
    MenuButton* achievementsButton = nullptr;
    MenuButton* ranklistButton = nullptr;
    MenuButton* settingsButton = nullptr;
    MenuButton* aboutButton = nullptr;
    MenuButton* exitButton = nullptr;

    QHBoxLayout* mainLayout = nullptr;
    QVBoxLayout* leftLayout = nullptr;
    QWidget* view3DContainer = nullptr;

    QPixmap backgroundPixmap;
    bool hasBackground = false;

    Qt3DExtras::Qt3DWindow* view3D = nullptr;
    Qt3DCore::QEntity* rootEntity = nullptr;
    Qt3DCore::QEntity* octahedronEntity = nullptr;
    Qt3DCore::QTransform* octahedronTransform = nullptr;
    QPropertyAnimation* rotationAnim = nullptr;

    void setupUI();
    void setup3DView();
protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
};
#endif // MENU_WIDGET_H
