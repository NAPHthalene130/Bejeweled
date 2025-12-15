#ifndef ACHIEVEMENTITEMWIDGET_H
#define ACHIEVEMENTITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include "game/data/AchievementData.h"

class AchievementItemWidget : public QWidget {
    Q_OBJECT

public:
    explicit AchievementItemWidget(const AchievementData& data, QWidget* parent = nullptr);

    void updateDisplay(const AchievementData& data);

private:
    QLabel* m_titleLabel;
    QLabel* m_contentLabel;

    void applyStyle(bool isUnlocked);
};

#endif // ACHIEVEMENTITEMWIDGET_H