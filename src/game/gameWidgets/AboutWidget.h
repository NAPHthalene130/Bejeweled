#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QVector>

class GameWindow;

class AboutWidget : public QWidget {
    Q_OBJECT
public:
    explicit AboutWidget(QWidget *parent = nullptr, GameWindow *gameWindow = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

signals:
    void backToMenu();

private:
    GameWindow *gameWindow;
    QWidget *contentBox;
    QTextEdit *textDisplay;
    QPushButton *backButton;

    // Animation related
    QPropertyAnimation *entryAnimation;
    QGraphicsOpacityEffect *opacityEffect;
    float bgHue = 0.0f;
    
    QPixmap backgroundPixmap;

    // Wind & Sakura Animation
    struct SakuraParticle {
        float x, y;
        float speedX, speedY;
        float rotation;
        float rotationSpeed;
        float size;
        float opacity;
    };
    QVector<SakuraParticle> sakuras;
    float time = 0.0f; // For sine wave calculations

    void setupUI();
    void setupAnimations();
    void updateSakuras();
};

#endif // ABOUTWIDGET_H
