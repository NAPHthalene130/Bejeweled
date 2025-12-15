#ifndef ACHIEVEMENTSWIDGET_H
#define ACHIEVEMENTSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <algorithm>
#include <vector>
#include "game/data/AchievementData.h"
#include "components/AchievementItemWidget.h"

// GameWindow 的前置声明 (假设您的主窗口类叫 GameWindow)
class GameWindow {
public:
    // 这是 AchievementsWidget 依赖的关键接口
    const std::vector<AchievementData>& getAchievementsContainer() const;
};

class AchievementsWidget : public QWidget {
    Q_OBJECT // 关键：解决 vtable 链接错误

public:
    explicit AchievementsWidget(GameWindow* gameWindow, QWidget* parent = nullptr);

public slots:
    void updateAchievementsWidget();

private slots:
    void goToNextPage();
    void goToPreviousPage();

private:
    GameWindow* m_gameWindow;
    std::vector<AchievementData> m_currentAchievements;
    
    int m_currentPage = 0;
    const int ACHIEVEMENTS_PER_PAGE = 8;
    int m_totalPages = 0;

    QVBoxLayout* m_mainLayout;
    QGridLayout* m_achievementsGrid;
    QLabel* m_pageInfoLabel;
    QPushButton* m_prevButton;
    QPushButton* m_nextButton;

    void setupUI();
    void loadPage(int pageIndex);
    void calculateTotalPages();
};

#endif // ACHIEVEMENTSWIDGET_H