#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <QObject>
#include <QSoundEffect>

class AudioManager : public QObject {
    Q_OBJECT
public:
    static AudioManager& instance();
    
    void playHoverSound();
    void playClickSound();

private:
    AudioManager();
    ~AudioManager();
    
    QSoundEffect* hoverSound;
    QSoundEffect* clickSound;
};

#endif // AUDIO_MANAGER_H
