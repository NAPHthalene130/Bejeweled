#ifndef ACHIEVEMENTDATA_H
#define ACHIEVEMENTDATA_H

#include <QString>

class AchievementData {
public:
    AchievementData(const QString& title, const QString& content, bool isUnlocked = false);

    QString getTitle() const;
    QString getContent() const;
    bool isUnlocked() const;

    void setTitle(const QString& title);
    void setContent(const QString& content);
    void setUnlocked(bool unlocked);

private:
    QString m_title;
    QString m_content;
    bool m_isUnlocked;
};

#endif // ACHIEVEMENTDATA_H
