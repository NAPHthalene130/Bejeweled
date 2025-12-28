// SettingWidget.h
#ifndef SETTINGWIDGET_H
#define SETTINGWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTimer>
#include <QList>
#include <QPointF>
#include <QSizeF>
#include <QPainterPath>

struct Particle {
    QPointF pos;       // 位置（相对于窗口比例）
    QSizeF size;       // 尺寸
    qreal angle;       // 旋转角度
    qreal speed;       // 移动速度
    qreal angleSpeed;  // 旋转速度
    qreal opacity;     // 透明度
    qreal driftX;      // X轴漂移系数
    qreal driftY;      // Y轴漂移系数
    int type;          // 0=橙叶 1=云絮
};

class GameWindow;

class SettingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingWidget(QWidget *parent = nullptr, GameWindow *gameWindow = nullptr);
    
    // 静态获取设置方法
    static int getBackgroundMusicVolume();
    static int getEliminateSoundVolume();
    static bool isBackgroundMusicEnabled();
    static bool isEliminateSoundEnabled();
    static QString getMenuBackgroundImage();
    static QString getGemStyle();  // 新增：获取宝石风格

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void saveSettings();
    void selectMenuBackground();
    void updateAnimation();
    void onGemStyleChanged(int index);  // 新增：宝石风格变化槽

private:
    void initParticles();
    void loadSettings();
    void updateGemStyleComboItems();  // 新增：更新可用风格列表
    
    GameWindow *gameWindow;
    QSettings *settings;
    qreal animTime;
    QTimer *animTimer;
    QList<Particle> particles;
    QString currentBgPath;
    
    // 音乐设置控件
    QSlider *bgMusicSlider;
    QSlider *eliminateSoundSlider;
    QCheckBox *bgMusicEnableBox;
    QCheckBox *eliminateSoundEnableBox;
    QLabel *bgVolLabel;
    QLabel *eliminateVolLabel;
    QComboBox *eliminateSoundCombo;
    QLabel *eliminateSoundSelectLabel;
    
    // 图像设置控件
    QComboBox *resolutionCombo;
    QComboBox *qualityCombo;
    QPushButton *selectBgBtn;
    QLabel *bgPreviewLabel;
    
    // 游戏设置控件
    QPushButton *switchInterfaceBtn;
    QComboBox *gemStyleCombo;
    QLabel *gemStyleDescLabel;  // 新增：风格描述标签
    
    // 按钮
    QPushButton *saveBtn;
    QPushButton *backBtn;
    
    // 标签页
    QTabWidget *tabWidget;

signals:
    void backgroundImageChanged(const QString &path);
    void gemStyleChanged(const QString &styleName);  // 新增：宝石风格变化信号
};

#endif // SETTINGWIDGET_H
