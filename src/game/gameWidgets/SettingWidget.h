#ifndef SETTING_WIDGET_H
#define SETTING_WIDGET_H
#include <QWidget>
#include <QTabWidget>
#include <QSlider>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QSettings>
#include <QPushButton>
#include <QPaintEvent>

class GameWindow;
class SettingWidget : public QWidget {
    Q_OBJECT
public:
    explicit SettingWidget(QWidget* parent = nullptr, GameWindow* gameWindow = nullptr);
    // 提供设置读取接口，供其他组件调用
    static int getBackgroundMusicVolume();
    static int getEliminateSoundVolume();
    static bool isBackgroundMusicEnabled();
    static bool isEliminateSoundEnabled();
    static QString getMenuBackgroundImage();
signals:
    void backgroundImageChanged(const QString& imagePath); // 背景图变更信号
    void gemStyleChanged(const QString& style);
private slots:
    void saveSettings(); // 保存设置
    void loadSettings(); // 加载设置
    void selectMenuBackground(); // 选择菜单背景图
protected:
    // 新增：声明paintEvent重写函数（QWidget的虚函数，需要protected声明）
    void paintEvent(QPaintEvent* event) override;
private:
    GameWindow* gameWindow = nullptr;
    QSettings* settings; // 保存设置到配置文件
    // 音乐设置
    QSlider* bgMusicSlider;
    QSlider* eliminateSoundSlider;
    QCheckBox* bgMusicEnableBox;
    QCheckBox* eliminateSoundEnableBox;
    // 图像设置
    QComboBox* resolutionCombo;
    QComboBox* qualityCombo;
    QPushButton* selectBgBtn;
    QLabel* bgPreviewLabel;
    QString currentBgPath;
    // 游戏设置（预留接口）
    QPushButton* switchInterfaceBtn;
    QLabel* gemStyleLabel; // 宝石风格标签
    QComboBox* gemStyleCombo; // 宝石风格选择下拉框
    QPushButton* backButton; // 返回菜单按钮
};
#endif // SETTING_WIDGET_H