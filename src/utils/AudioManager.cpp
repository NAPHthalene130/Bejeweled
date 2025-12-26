#include "AudioManager.h"
#include <QUrl>
#include <string>
#include <QCoreApplication>
#include <QDir>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QElapsedTimer>
#include <QDialog>
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


void AudioManager::playEliminateSound(int comboCount) {
    // 1. 检查设置中是否启用消除音效，未启用则直接返回
    if (!SettingWidget::isEliminateSoundEnabled()) return;

    // 2. 从设置中获取用户选择的音效类型（默认值为"Manbo"）
    // 建议：如果SettingWidget有封装好的getEliminateSoundType()接口，可替换此处的QSettings直接调用，更符合封装原则
    QSettings settings("GemMatch", "Settings");
    QString soundType = settings.value("Music/EliminateType", "Manbo").toString();
    // 将QString转为std::string，方便拼接文件名
    std::string soundTypeStr = soundType.toStdString();

    // 3. 根据连续消除次数选择音效的数字后缀（1-5）
    int soundSuffix = 1; // 默认后缀为1
    if (comboCount <= 3) {
        // 普通消除：使用1/2/3后缀
        soundSuffix = comboCount;
    } else {
        // 连续消除：超过3次则循环使用4/5后缀
        soundSuffix = (comboCount % 2 == 0) ? 4 : 5;
    }

    // 4. 拼接最终的音效文件路径（如"sounds/Manbo3.wav"）
    // 移除对Manbo子目录的特殊处理，统一使用 resources/sounds/ 下的平铺结构
    std::string soundFile = "sounds/" + soundTypeStr + std::to_string(soundSuffix) + ".wav";

    // 5. 初始化音效播放器并设置资源路径
    QSoundEffect* eliminateSound = new QSoundEffect(this);
    std::string eliminatePath = ResourceUtils::getPath(soundFile);
    eliminateSound->setSource(QUrl::fromLocalFile(QString::fromStdString(eliminatePath)));
    
    // Debug: 输出音频路径和加载状态
    qDebug() << "Trying to play eliminate sound:" << QString::fromStdString(eliminatePath);
    
    // 设置默认音频输出设备
    QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (!device.isNull()) {
        eliminateSound->setAudioDevice(device);
    }

    // 6. 应用设置中的音量（确保音量在0-100范围内，再转为0.0-1.0的浮点数）
    int vol = qBound(0, SettingWidget::getEliminateSoundVolume(), 100);
    eliminateSound->setVolume(vol / 100.0f);

    // 7. 播放音效
    // 连接信号以监控状态和自动清理
    connect(eliminateSound, &QSoundEffect::statusChanged, this, [eliminateSound]() {
        if (eliminateSound->status() == QSoundEffect::Error) {
            qDebug() << "Sound effect error:" << eliminateSound->source();
        }
    });

    // 8. 播放完成后自动释放资源，避免内存泄漏
    connect(eliminateSound, &QSoundEffect::playingChanged, [eliminateSound]() {
        if (!eliminateSound->isPlaying()) {
            eliminateSound->deleteLater();
        }
    });

    eliminateSound->play();
}


