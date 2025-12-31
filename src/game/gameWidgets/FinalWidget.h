#ifndef FINAL_WIDGET_H
#define FINAL_WIDGET_H

#include <QWidget>
#include <QPixmap>
#include <string>

class GameWindow;
class QLabel;
class MenuButton;
class QVBoxLayout;
class QPaintEvent;
class QResizeEvent;

class FinalWidget : public QWidget {
public:
    explicit FinalWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    void setTitleStr(std::string str);
    void setContentStr(std::string str);
    // Deprecated, use setContentStr instead
    void setGradeContent(std::string str);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void loadBackground();

    GameWindow* gameWindow = nullptr;

    QPixmap backgroundPixmap;
    bool hasBackground = false;

    QVBoxLayout* mainLayout = nullptr;
    QWidget* panelWidget = nullptr;
    QLabel* titleLabel = nullptr;
    QLabel* gradeLabel = nullptr;
    MenuButton* backButton = nullptr;
};

#endif // FINAL_WIDGET_H
