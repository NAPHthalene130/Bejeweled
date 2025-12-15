#include "game/data/AchievementData.h"  

// ���캯����ʵ��
AchievementData::AchievementData(const QString& title, const QString& content, bool isUnlocked)
    : m_title(title), m_content(content), m_isUnlocked(isUnlocked) {}

// Getters ʵ��
QString AchievementData::getTitle() const {
    return m_title;
}

QString AchievementData::getContent() const {
    return m_content;
}

bool AchievementData::isUnlocked() const {
    return m_isUnlocked;
}

// Setters ʵ��
void AchievementData::setTitle(const QString& title) {
    m_title = title;
}

void AchievementData::setContent(const QString& content) {
    m_content = content;
}

void AchievementData::setUnlocked(bool unlocked) {
    m_isUnlocked = unlocked;
}
