#ifndef ACHIEVEMENTS_WIDGET_H
#define ACHIEVEMENTS_WIDGET_H
#include <QWidget>

class GameWindow;
class QVBoxLayout;
class QGridLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QPixmap;

#ifdef HAVE_QT3D
namespace Qt3DExtras { class Qt3DWindow; }
namespace Qt3DCore { class QEntity; }
#endif

class AchievementsWidget : public QWidget {
    Q_OBJECT
public:
    explicit AchievementsWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    // Force UI to reload achievements from GameWindow
    void updateView();

private slots:
    void onPrevPage();
    void onNextPage();

private:
    void refresh();

    GameWindow* gameWindow = nullptr;

    QVBoxLayout* mainLayout = nullptr;
    QWidget* gridContainer = nullptr;
    QGridLayout* gridLayout = nullptr;
    QPixmap bgPixmap;
    QLabel* bgLabel = nullptr;

// protected:
//     void resizeEvent(QResizeEvent* event) override;
#ifdef HAVE_QT3D
    // Qt3D background (optional)
    Qt3DExtras::Qt3DWindow* qt3dWindow = nullptr;
    QWidget* qt3dContainer = nullptr;
    Qt3DCore::QEntity* qt3dRoot = nullptr;
#endif
    QHBoxLayout* navLayout = nullptr;
    QPushButton* prevButton = nullptr;
    QPushButton* nextButton = nullptr;
    QLabel* pageLabel = nullptr;

    int currentPage = 0;
    int itemsPerPage = 5; // 3 columns x 2 rows (first row 3, second row 2 centered)
protected:
    void resizeEvent(QResizeEvent* event) override;
    void updateBackground();
};
#endif // ACHIEVEMENTS_WIDGET_H

