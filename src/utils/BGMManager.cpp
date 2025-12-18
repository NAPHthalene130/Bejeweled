#include "BGMManager.h"
#include <QUrl>
#include <QDebug>
#include "../game/gameWidgets/SettingWidget.h"
#include "ResourceUtils.h"

// 单例实现
BGMManager& BGMManager::instance() {
    static BGMManager instance; // 局部静态变量，使用默认参数nullptr，无需传参
    return instance;
}

// 构造函数实现（现在和头文件声明匹配）
BGMManager::BGMManager(QObject* parent) : QObject(parent) {
    player = new QMediaPlayer(this); // 父对象设为this，自动管理内存
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    // 从设置中获取初始音量和状态
    int volume = SettingWidget::getBackgroundMusicVolume();
    setVolume(volume);

    bool enabled = SettingWidget::isBackgroundMusicEnabled();
    if (!enabled) {
        audioOutput->setMuted(true);
    }
}

// 析构函数
BGMManager::~BGMManager() {
    player->stop();
    // QObject的子对象会被自动销毁，无需手动delete
}

// 以下原有函数实现保持不变（播放、停止、暂停、恢复、设置音量）
void BGMManager::play(const QString& filePath) {
    if (!SettingWidget::isBackgroundMusicEnabled()) {
        return;
    }

    if (currentFilePath == filePath && player->playbackState() == QMediaPlayer::PlayingState) {
        return;
    }

    currentFilePath = filePath;
    player->setSource(QUrl::fromLocalFile(filePath));
    player->setLoops(QMediaPlayer::Infinite); // 循环播放
    player->play();
}

void BGMManager::stop() {
    player->stop();
    currentFilePath.clear();
}

void BGMManager::pause() {
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
    }
}

void BGMManager::resume() {
    if (SettingWidget::isBackgroundMusicEnabled() &&
        player->playbackState() == QMediaPlayer::PausedState) {
        player->play();
    }
}

void BGMManager::setVolume(int volume) {
    volume = qBound(0, volume, 100);
    audioOutput->setVolume(volume / 100.0f);
}
