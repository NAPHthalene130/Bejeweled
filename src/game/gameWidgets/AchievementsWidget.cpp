#include "AchievementsWidget.h"
#include "../GameWindow.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QFont>
#include <QMessageBox>
#include <QTimer>
#include <QFileInfo>
#include <QPixmap>
#include <QStackedLayout>
#include <QSizePolicy>
#include <QUrl>
#include <QVector3D>
#include <QGraphicsDropShadowEffect>
#ifdef HAVE_QT3D
#include <Qt3DExtras/qt3dwindow.h>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>
#include <Qt3DExtras/qplanemesh.h>
#include <Qt3DRender/qtexture.h>
#include <Qt3DRender/qtextureimage.h>
#include <Qt3DExtras/qtexturematerial.h>
#include <Qt3DRender/qcamera.h>
#include <QWidget>
#endif

// Small widget to represent a single achievement
class AchievementItem : public QFrame {
public:
    AchievementItem(const AchievementData& a, QWidget* parent = nullptr) : QFrame(parent), data(a) {
        setFrameShape(QFrame::StyledPanel);
        setLineWidth(1);
        setContentsMargins(8,8,8,8);
        setMinimumSize(280, 100);
        setMaximumSize(360, 140);
        setFixedSize(300, 110);
        QVBoxLayout* l = new QVBoxLayout(this);
        titleLabel = new QLabel(data.getTitle(), this);
        QFont ft = titleLabel->font(); ft.setBold(true); ft.setPointSize(12); titleLabel->setFont(ft);
        descLabel = new QLabel(data.getDescription(), this);
        descLabel->setWordWrap(true);
        statusLabel = new QLabel(this);
        statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        applyStyle();

        // add a subtle drop shadow so cards stand out from the colored background
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(10);
        shadow->setOffset(0, 3);
        shadow->setColor(QColor(0,0,0,120));
        setGraphicsEffect(shadow);

        QHBoxLayout* top = new QHBoxLayout();
        top->addWidget(titleLabel);
        top->addWidget(statusLabel, 0, Qt::AlignRight);
        l->addLayout(top);
        l->addWidget(descLabel);
    }

protected:
    void mousePressEvent(QMouseEvent* ev) override {
        Q_UNUSED(ev)
        // visual feedback
        QString prev = styleSheet();
        setStyleSheet(prev + "border: 2px solid rgba(255,255,255,0.15); transform: translateY(2px);");
        QTimer::singleShot(150, this, [this, prev]() { setStyleSheet(prev); });

        // show completion time or locked message
        if (data.isUnlocked()) {
            QDateTime dt = data.getCompletedAt();
            QString t = dt.isValid() ? dt.toString("yyyy-MM-dd hh:mm:ss") : QString("未知时间");
            QMessageBox::information(this, data.getTitle(), QString("已于 %1 解锁").arg(t));
        } else {
            QMessageBox::information(this, data.getTitle(), QString("尚未解锁"));
        }
    }

private:
    void applyStyle() {
            // color accents according to difficulty
        QString accent;
        switch (data.getDifficulty()) {
            case AchievementData::Difficulty::Easy: accent = "#4a90e2"; break; // blue
            case AchievementData::Difficulty::Medium: accent = "#ff8c00"; break; // orange
            case AchievementData::Difficulty::Hard: accent = "#d14949"; break; // red
            case AchievementData::Difficulty::Ultimate: accent = "#706bce"; break; // purple
        }
        if (data.isUnlocked()) {
            // lighter card for unlocked so text is readable on colored background
            setStyleSheet(QString("background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(255,255,255,0.95), stop:1 rgba(245,245,245,0.95)); color: #111; border-radius:8px; border-left:6px solid %1;").arg(accent));
            statusLabel->setText("已解锁");
            statusLabel->setStyleSheet("color: #007a39; font-weight:700;");
        } else {
            // dark card for locked so text is clear on the colored page background
            setStyleSheet(QString("background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(34,34,34,230), stop:1 rgba(18,18,18,230)); color: white; border-radius:8px; border-left:6px solid %1;").arg(accent));
            statusLabel->setText("未解锁");
            statusLabel->setStyleSheet("color: #ffffff; font-weight:600;");
        }
    }

    AchievementData data;
    QLabel* titleLabel;
    QLabel* descLabel;
    QLabel* statusLabel;
};

AchievementsWidget::AchievementsWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    // 用堆叠布局，底层为背景图，顶层为内容
    QStackedLayout* stack = new QStackedLayout(this);
    stack->setStackingMode(QStackedLayout::StackAll);

    // 背景图片label
    bgLabel = new QLabel(this);
    bgLabel->setAlignment(Qt::AlignCenter);
    bgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QString assetsPath = QString(PROJECT_SOURCE_DIR) + "/assets/achievements_bg.png";
    QPixmap pix(assetsPath);
    if (!pix.isNull()) {
        bgPixmap = pix;
        bgLabel->setPixmap(bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
    stack->addWidget(bgLabel);

    // 内容层，设置为不透明白色，防止被背景图片影响
    QWidget* contentWidget = new QWidget(this);
    contentWidget->setAutoFillBackground(true);
    QPalette pal = contentWidget->palette();
    pal.setColor(QPalette::Window, QColor(255,255,255,255));
    contentWidget->setPalette(pal);
    mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(24,24,24,24);
    mainLayout->setSpacing(12);

    QLabel* header = new QLabel("成就", contentWidget);
    QFont hf = header->font(); hf.setPointSize(18); hf.setBold(true); header->setFont(hf);
    mainLayout->addWidget(header);

    QScrollArea* scroll = new QScrollArea(contentWidget);
    scroll->setWidgetResizable(true);
    gridContainer = new QWidget();
    gridLayout = new QGridLayout(gridContainer);
    gridLayout->setSpacing(12);
    gridLayout->setContentsMargins(0,0,0,0);
    scroll->setWidget(gridContainer);
    mainLayout->addWidget(scroll, 1);

    // 透明滚动区，卡片自身不透明
    scroll->setStyleSheet("background: transparent;");
    gridContainer->setStyleSheet("background: transparent;");

    navLayout = new QHBoxLayout();
    prevButton = new QPushButton("上一页", contentWidget);
    nextButton = new QPushButton("下一页", contentWidget);
    pageLabel = new QLabel(contentWidget);
    navLayout->addWidget(prevButton);
    navLayout->addStretch(1);
    navLayout->addWidget(pageLabel);
    navLayout->addStretch(1);
    navLayout->addWidget(nextButton);
    mainLayout->addLayout(navLayout);

    stack->addWidget(contentWidget);
    setLayout(stack);

    connect(prevButton, &QPushButton::clicked, this, &AchievementsWidget::onPrevPage);
    connect(nextButton, &QPushButton::clicked, this, &AchievementsWidget::onNextPage);

    refresh();
}
void AchievementsWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (!bgPixmap.isNull() && bgLabel) {
        bgLabel->setPixmap(bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
}

void AchievementsWidget::onPrevPage() {
    if (currentPage > 0) {
        currentPage--;
        refresh();
    }
}

void AchievementsWidget::onNextPage() {
    int total = (int)gameWindow->getAchievements().size();
    int pages = (total + itemsPerPage - 1) / itemsPerPage;
    if (currentPage + 1 < pages) {
        currentPage++;
        refresh();
    }
}

void AchievementsWidget::updateView() {
    // reset to first page when updating view
    currentPage = 0;
    refresh();
}

void AchievementsWidget::refresh() {
    // clear grid
    QLayoutItem* child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    const auto& list = gameWindow->getAchievements();
    int total = (int)list.size();
    int pages = qMax(1, (total + itemsPerPage - 1) / itemsPerPage);

    int start = currentPage * itemsPerPage;
    int end = qMin(total, start + itemsPerPage);

    int colCount = 3; // 3 columns; second row will place 2 items centered (cols 0 and 2)
    int row = 0, col = 0;
    for (int idx = 0; idx < end - start; ++idx) {
        const AchievementData& a = list[start + idx];
        AchievementItem* item = new AchievementItem(a, gridContainer);
        if (idx < 3) {
            row = 0;
            col = idx; // 0,1,2 on first row
        } else {
            row = 1;
            // place two items on second row centered: cols 0 and 2
            col = (idx - 3 == 0) ? 0 : 2;
        }
        gridLayout->addWidget(item, row, col);
    }

    pageLabel->setText(QString("%1 / %2 页").arg(currentPage+1).arg(pages));

    prevButton->setEnabled(currentPage > 0);
    nextButton->setEnabled(currentPage + 1 < pages);
}

// 移除旧的 resizeEvent 和 updateBackground，避免重定义
