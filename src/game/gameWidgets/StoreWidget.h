#ifndef STORE_WIDGET_H
#define STORE_WIDGET_H
#include <QWidget>

class GameWindow;
class QLabel;
class QPushButton;
class QVBoxLayout;

class StoreWidget : public QWidget {
    Q_OBJECT
public:
    explicit StoreWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);

public slots:
    void updateCoinDisplay(int newCoins);

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void loadBackground();

    GameWindow* gameWindow = nullptr;
    QLabel* coinLabel = nullptr;
    QWidget* mainPanel = nullptr;
};
#endif // STORE_WIDGET_H
