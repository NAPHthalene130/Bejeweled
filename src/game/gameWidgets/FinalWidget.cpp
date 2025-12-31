#include "FinalWidget.h"
#include "MenuWidget.h"
#include "../GameWindow.h"
#include "../components/MenuButton.h"
#include "../../utils/BackgroundManager.h"
#include "../../utils/ResourceUtils.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>
#include <QFile>

FinalWidget::FinalWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    setupUI();
    loadBackground();
}

void FinalWidget::setTitleStr(std::string str) {
    if (!titleLabel) return;
    titleLabel->setText(QString::fromStdString(str));
}

void FinalWidget::setContentStr(std::string str) {
    if (!gradeLabel) return;
    gradeLabel->setText(QString::fromStdString(str));
}

void FinalWidget::setGradeContent(std::string str) {
    setContentStr(str);
}

void FinalWidget::setupUI() {
    setMinimumSize(1280, 720);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(80, 60, 80, 60);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignCenter);

    panelWidget = new QWidget(this);
    panelWidget->setMinimumSize(760, 560);
    panelWidget->setMaximumWidth(900);
    panelWidget->setStyleSheet(R"(
        QWidget {
            background-color: rgba(20, 24, 40, 165);
            border: 1px solid rgba(255, 255, 255, 40);
            border-radius: 22px;
        }
    )");

    auto* panelShadow = new QGraphicsDropShadowEffect(panelWidget);
    panelShadow->setBlurRadius(40);
    panelShadow->setOffset(0, 14);
    panelShadow->setColor(QColor(0, 0, 0, 140));
    panelWidget->setGraphicsEffect(panelShadow);

    auto* panelLayout = new QVBoxLayout(panelWidget);
    panelLayout->setContentsMargins(56, 44, 56, 44);
    panelLayout->setSpacing(28);

    titleLabel = new QLabel("本局成绩", panelWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(36);
    titleFont.setBold(true);
    titleFont.setFamily("Segoe UI");
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    titleLabel->setStyleSheet("color: #FFD700; background: transparent;");

    QWidget* gradeCard = new QWidget(panelWidget);
    gradeCard->setStyleSheet(R"(
        QWidget {
            background-color: rgba(255, 255, 255, 35);
            border: 1px solid rgba(255, 255, 255, 55);
            border-radius: 18px;
        }
    )");
    auto* gradeShadow = new QGraphicsDropShadowEffect(gradeCard);
    gradeShadow->setBlurRadius(22);
    gradeShadow->setOffset(0, 8);
    gradeShadow->setColor(QColor(0, 0, 0, 120));
    gradeCard->setGraphicsEffect(gradeShadow);

    auto* gradeLayout = new QVBoxLayout(gradeCard);
    gradeLayout->setContentsMargins(26, 22, 26, 22);
    gradeLayout->setSpacing(10);

    gradeLabel = new QLabel(gradeCard);
    QFont gradeFont = gradeLabel->font();
    gradeFont.setPointSize(14);
    gradeFont.setBold(true);
    gradeFont.setFamily("Segoe UI");
    gradeLabel->setFont(gradeFont);
    gradeLabel->setAlignment(Qt::AlignCenter);
    gradeLabel->setWordWrap(true);
    gradeLabel->setStyleSheet("color: white; background: transparent; line-height: 1.4;");
    gradeLayout->addWidget(gradeLabel);

    backButton = new MenuButton(220, 60, 18, QColor(120, 220, 255), "返回主菜单", panelWidget);
    backButton->setAttribute(Qt::WA_NativeWindow);

    connect(backButton, &QPushButton::clicked, this, [this]() {
        if (gameWindow && gameWindow->getMenuWidget()) {
            gameWindow->switchWidget(gameWindow->getMenuWidget());
        }
    });

    panelLayout->addWidget(titleLabel, 0, Qt::AlignHCenter | Qt::AlignTop);
    panelLayout->addWidget(gradeCard, 1);
    panelLayout->addStretch(1);
    panelLayout->addWidget(backButton, 0, Qt::AlignHCenter | Qt::AlignBottom);

    mainLayout->addWidget(panelWidget, 0, Qt::AlignCenter);
}

void FinalWidget::loadBackground() {
    QString bgPath = QString::fromStdString(
        ResourceUtils::getPath(BackgroundManager::instance().getFinalWidgetBackground())
    );
    if (QFile::exists(bgPath)) {
        backgroundPixmap.load(bgPath);
        hasBackground = !backgroundPixmap.isNull();
    } else {
        hasBackground = false;
    }
    update();
}

void FinalWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (hasBackground) {
        QPixmap scaled = backgroundPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        p.drawPixmap(0, 0, scaled);
    } else {
        p.fillRect(rect(), QColor(8, 10, 18));
    }

    p.fillRect(rect(), QColor(0, 0, 0, 85));
}

void FinalWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    update();
}
