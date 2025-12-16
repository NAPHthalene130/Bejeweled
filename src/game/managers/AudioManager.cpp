#include "AudioManager.h"
#include <QUrl>
#include <QCoreApplication>
#include <QDir>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QElapsedTimer>

AudioManager& AudioManager::instance() {
    static AudioManager _instance;
    return _instance;
}

AudioManager::AudioManager() : QObject(nullptr), lastHoverPlayTime(0) {
    throttleTimer.start();
    
    QString soundDir;
#ifdef PROJECT_SOURCE_DIR
    soundDir = QString(PROJECT_SOURCE_DIR) + "/resources/sounds/";
#else
    // Fallback if macro is not defined (e.g. running from build dir)
    soundDir = QCoreApplication::applicationDirPath() + "/resources/sounds/";
#endif

    hoverSound = new QSoundEffect(this);
    hoverSound->setSource(QUrl::fromLocalFile(soundDir + "MenuButtonHover.wav"));
    hoverSound->setVolume(0.5f);

    clickSound = new QSoundEffect(this);
    clickSound->setSource(QUrl::fromLocalFile(soundDir + "MenuButtonClicked.wav"));
    clickSound->setVolume(0.5f);

    // Monitor audio device changes
    mediaDevices = new QMediaDevices(this);
    connect(mediaDevices, &QMediaDevices::audioOutputsChanged, this, &AudioManager::updateAudioOutput);
    
    // Set initial device
    updateAudioOutput();
}

AudioManager::~AudioManager() {
}

void AudioManager::updateAudioOutput() {
    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (!device.isNull()) {
        hoverSound->setAudioDevice(device);
        clickSound->setAudioDevice(device);
    }
}

void AudioManager::playHoverSound() {
    // Throttle to prevent freezing on rapid events
    qint64 now = throttleTimer.elapsed();
    if (now - lastHoverPlayTime < 100) { // Limit to 10 sounds per second max
        return;
    }
    lastHoverPlayTime = now;

    if (hoverSound->status() == QSoundEffect::Ready || hoverSound->status() == QSoundEffect::Loading) {
        hoverSound->play();
    }
}

void AudioManager::playClickSound() {
    if (clickSound->status() == QSoundEffect::Ready || clickSound->status() == QSoundEffect::Loading) {
        clickSound->play();
    }
}
