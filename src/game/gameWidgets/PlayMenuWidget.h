#ifndef PLAY_MENU_WIDGET_H
#define PLAY_MENU_WIDGET_H

#include <QWidget>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <QVBoxLayout>

class GameWindow;
class MenuButton;

class PlayMenuWidget : public QWidget {
    Q_OBJECT
public:
    explicit PlayMenuWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    GameWindow* gameWindow;
    QVBoxLayout* mainLayout;
    QVBoxLayout* centerLayout;
    
    MenuButton* normalModeButton;
    MenuButton* rotateModeButton;
    MenuButton* multiModeButton;
    MenuButton* testMultiButton;
    MenuButton* puzzleModeButton;
    MenuButton* backButton;

    // 3D View
    QWidget* view3DContainer;
    Qt3DExtras::Qt3DWindow* view3D;
    Qt3DCore::QEntity* rootEntity;

    void setupUI();
    void setup3DView();

private slots:
    void multiModeButtonClicked();

signals:
    void backToMenu();
    void startNormalMode();
    void startRotateMode();
};

#endif // PLAY_MENU_WIDGET_H
