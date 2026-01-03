#ifndef STORE_WIDGET_H
#define STORE_WIDGET_H
#include <QWidget>
#include <map>
#include "../data/ItemSystem.h"
#include <QTimer>           // 新增
#include <QPropertyAnimation> // 新增

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
    void showFloatingMessage(const QString& text, bool isSuccess); // 新增声明
    void removeFloatingMessage(QLabel* label); // 新增辅助函数

    GameWindow* gameWindow = nullptr;
    QLabel* coinLabel = nullptr;
    QWidget* mainPanel = nullptr;

    // 道具显示标签（显示拥有数量）
    std::map<ItemType, QLabel*> itemCountLabels;
};
#endif // STORE_WIDGET_H