#include "AudioManager.h"
#include <QUrl>
#include <QCoreApplication>
#include <QDir>

AudioManager& AudioManager::instance() {
    static AudioManager _instance;
    return _instance;
}

AudioManager::AudioManager() : QObject(nullptr) {
    // Assuming PROJECT_SOURCE_DIR is defined via build system, 
    // but if not, we might need a fallback or use relative paths.
    // MenuButton used PROJECT_SOURCE_DIR, so we will use it too if available.
    // If this file is compiled where PROJECT_SOURCE_DIR is not defined, we might have an issue.
    // However, since it's in the same project, it should be fine.
    
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
}

AudioManager::~AudioManager() {
    // Parent is nullptr, so we should delete children or let QObject handle it if we parented them.
    // We parented them to 'this' in constructor.
    // But since this is a singleton, it might be destroyed at app exit.
}

void AudioManager::playHoverSound() {
    if (hoverSound->status() == QSoundEffect::Ready || hoverSound->status() == QSoundEffect::Loading) {
        hoverSound->play();
    }
}

void AudioManager::playClickSound() {
    if (clickSound->status() == QSoundEffect::Ready || clickSound->status() == QSoundEffect::Loading) {
        clickSound->play();
    }
}
