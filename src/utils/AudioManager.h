#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <QObject>
#include <QSoundEffect>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QElapsedTimer>
#include <QRandomGenerator> 

class AudioManager : public QObject {
    Q_OBJECT
public:
    static AudioManager& instance();
    
    void playHoverSound();
    void playClickSound();
    void playEliminateSound(int comboCount = 1);

private slots:
    void updateAudioOutput();

private:
    AudioManager();
    ~AudioManager();
    
    QSoundEffect* hoverSound;
    QSoundEffect* clickSound;
    
    QMediaDevices* mediaDevices;
    QElapsedTimer throttleTimer;
    qint64 lastHoverPlayTime;
    QRandomGenerator m_randomGenerator;  
};

#endif // AUDIO_MANAGER_H
