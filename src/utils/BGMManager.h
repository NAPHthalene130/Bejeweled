#ifndef BGM_MANAGER_H
#define BGM_MANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QString>

class BGMManager : public QObject {
    Q_OBJECT  // 保留moc必需的宏
public:
    // 单例获取接口
    static BGMManager& instance();

    // 背景音乐控制接口
    void play(const QString& filePath);
    void stop();
    void pause();
    void resume();
    void setVolume(int volume); // 0-100范围

private:
    // ********** 关键修改：声明带参数且有默认值的构造函数 **********
    explicit BGMManager(QObject* parent = nullptr); // 带parent参数，默认值nullptr
    ~BGMManager(); // 析构函数（无需override，因为QObject的析构函数是虚函数，但这里可以省略）

    // 成员变量
    QMediaPlayer* player;
    QAudioOutput* audioOutput;
    QString currentFilePath;

    // 禁止复制和赋值
    BGMManager(const BGMManager&) = delete;
    BGMManager& operator=(const BGMManager&) = delete;
};

#endif // BGM_MANAGER_H