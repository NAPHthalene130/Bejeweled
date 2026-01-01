#ifndef STORE_WIDGET_H
#define STORE_WIDGET_H
#include <QWidget>
#include <map>
#include "../data/ItemSystem.h"

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
    void updateItemDisplay(ItemType type, int newCount);

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void loadBackground();
    QWidget* createItemCard(const ItemInfo& info);
    void onPurchaseClicked(ItemType type);

    GameWindow* gameWindow = nullptr;
    QLabel* coinLabel = nullptr;
    QWidget* mainPanel = nullptr;

    // 道具显示标签（显示拥有数量）
    std::map<ItemType, QLabel*> itemCountLabels;
};
#endif // STORE_WIDGET_H
