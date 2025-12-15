#include "game/data/AchievementData.h"
#include "components/AchievementItemWidget.h"
#include <QDebug>

AchievementItemWidget::AchievementItemWidget(const AchievementData& data, QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(5);
    
    m_titleLabel = new QLabel(this);
    m_contentLabel = new QLabel(this);
    
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    m_titleLabel->setFont(titleFont);
    
    m_contentLabel->setWordWrap(true);
    
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_contentLabel);
    
    setFixedSize(250, 100); 

    updateDisplay(data);
}

void AchievementItemWidget::updateDisplay(const AchievementData& data) {
    bool unlocked = data.isUnlocked();

    m_titleLabel->setText(data.getTitle());

    if (unlocked) {
        m_contentLabel->setText(data.getContent());
    } else {
        m_contentLabel->setText("??? - Locked - ???");
    }

    applyStyle(unlocked);
}

void AchievementItemWidget::applyStyle(bool isUnlocked) {
    if (isUnlocked) {
        setStyleSheet(
            "AchievementItemWidget {"
            "   border: 2px solid #FFD700;"
            "   border-radius: 8px;"
            "   background-color: #3C3C3C;"
            "}"
            "QLabel { color: white; }"
        );
    } else {
        setStyleSheet(
            "AchievementItemWidget {"
            "   border: 2px dashed #606060;"
            "   border-radius: 8px;"
            "   background-color: #252525;"
            "}"
            "QLabel { color: #888888; }"
        );
    }
}
