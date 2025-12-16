#ifndef ACHIEVEMENT_DATA_H
#define ACHIEVEMENT_DATA_H

#include <QString>
#include <QDateTime>

class AchievementData {
public:
    AchievementData() = default;
    AchievementData(const QString &title, const QString &description, bool unlocked = false)
        : title(title), description(description), unlocked(unlocked) {}

    const QString& getTitle() const { return title; }
    void setTitle(const QString &t) { title = t; }

    const QString& getDescription() const { return description; }
    void setDescription(const QString &d) { description = d; }

    bool isUnlocked() const { return unlocked; }
    void setUnlocked(bool u) { unlocked = u; }

    const QDateTime& getCompletedAt() const { return completedAt; }
    void setCompletedAt(const QDateTime& dt) { completedAt = dt; }

    enum class Difficulty {
        Easy,
        Medium,
        Hard,
        Ultimate
    };

    Difficulty getDifficulty() const { return difficulty; }
    void setDifficulty(Difficulty d) { difficulty = d; }

private:
    QString title;
    QString description;
    bool unlocked = false;
    QDateTime completedAt;
    Difficulty difficulty = Difficulty::Easy;
};

#endif // ACHIEVEMENT_DATA_H
