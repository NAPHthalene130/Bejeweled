#ifndef MULTIGAMEWAITWIDGET_H
#define MULTIGAMEWAITWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVariant>

namespace Qt3DExtras {
    class Qt3DWindow;
}
namespace Qt3DCore {
    class QEntity;
}

class MenuButton;
class GameWindow;

class MultiGameWaitWidget : public QWidget {
    Q_OBJECT

public:
    explicit MultiGameWaitWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    ~MultiGameWaitWidget();

    bool getIsInRoom() const;
    void setIsInRoom(bool value);

    int getRoomPeopleHave() const;
    void setRoomPeopleHave(int count);
    void resetRoomPeopleHaveLabel(int people);

    void enterRoom();
    void accept10(std::map<std::string, int> idToNum);

private slots:
    void backButtonClicked();

signals:
    void backToMenu();

private:
    GameWindow* gameWindow;
    void setupUI();
    void setup3DView();
    void updateInfoLabel();

    // UI Elements
    QWidget* view3DContainer;
    // QWidget* uiContainer; // Removed as we use main layout now
    Qt3DExtras::Qt3DWindow* view3D;
    Qt3DCore::QEntity* rootEntity;
    
    QLabel* infoLabel;
    MenuButton* backButton;

    // Data
    bool isInRoom;
    int roomPeopleHave;

protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif // MULTIGAMEWAITWIDGET_H
