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
    static int getBackgroundMusicVolume();
    static int getEliminateSoundVolume();
    static bool isBackgroundMusicEnabled();
    static bool isEliminateSoundEnabled();
    static QString getMenuBackgroundImage();
protected:
    void paintEvent(QPaintEvent *event) override;
private slots:
    void saveSettings();
    void selectMenuBackground();
    void updateAnimation();
private:
    void initParticles();
    void loadSettings();
    GameWindow *gameWindow;
    QSettings *settings;
    qreal animTime;
    QTimer *animTimer;
    QList<Particle> particles;
    QString currentBgPath;
    QSlider *bgMusicSlider;
    QSlider *eliminateSoundSlider;
    QCheckBox *bgMusicEnableBox;
    QCheckBox *eliminateSoundEnableBox;
    QLabel *bgVolLabel;
    QLabel *eliminateVolLabel;
    QComboBox *resolutionCombo;
    QComboBox *qualityCombo;
    QComboBox *gemStyleCombo;
    QPushButton *selectBgBtn;
    QLabel *bgPreviewLabel;
    QPushButton *switchInterfaceBtn;
    QPushButton *saveBtn;
    QPushButton *backBtn;
    QTabWidget *tabWidget;
    QComboBox *eliminateSoundCombo;
    QLabel *eliminateSoundSelectLabel;
signals:
    void backgroundImageChanged(const QString &path);
};
#endif // SETTINGWIDGET_H