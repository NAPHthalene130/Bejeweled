#include "AudioManager.h"
#include <QUrl>
#include <QCoreApplication>
#include <QDir>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QElapsedTimer>
#include "../game/gameWidgets/SettingWidget.h" 
#include "ResourceUtils.h"

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
    // 补充：检查静态函数返回值的合理性，同时判断音效对象是否为空
    if (!hoverSound || !SettingWidget::isEliminateSoundEnabled()) return;
    
    qint64 now = throttleTimer.elapsed();
    if (now - lastHoverPlayTime < 100) return;
    lastHoverPlayTime = now;
    
    // 补充：音量值范围检查（0-100）
    int vol = qBound(0, SettingWidget::getEliminateSoundVolume(), 100);
    hoverSound->setVolume(vol / 100.0f);
    
    if (hoverSound->status() == QSoundEffect::Ready || hoverSound->status() == QSoundEffect::Loading) {
        hoverSound->play();
    }
}

// playClickSound函数同理补充检查
void AudioManager::playClickSound() {
    if (!clickSound || !SettingWidget::isEliminateSoundEnabled()) return;
    
    int vol = qBound(0, SettingWidget::getEliminateSoundVolume(), 100);
    clickSound->setVolume(vol / 100.0f);
    
    if (clickSound->status() == QSoundEffect::Ready || clickSound->status() == QSoundEffect::Loading) {
        clickSound->play();
    }
}


// 新增消除音乐播放接口（供游戏调用）
void AudioManager::playEliminateSound() {
    if (!SettingWidget::isEliminateSoundEnabled()) return;
    // 假设消除音效文件为Eliminate.wav
    QSoundEffect* eliminateSound = new QSoundEffect(this);
    std::string eliminatePath = ResourceUtils::getPath("sounds/Eliminate.wav");
    eliminateSound->setSource(QUrl::fromLocalFile(QString::fromStdString(eliminatePath)));
    eliminateSound->setVolume(SettingWidget::getEliminateSoundVolume() / 100.0f);
    eliminateSound->play();
    // 播放完成后自动释放
    connect(eliminateSound, &QSoundEffect::playingChanged, [eliminateSound]() {
        if (!eliminateSound->isPlaying()) {
            eliminateSound->deleteLater();
        }
    });
}
