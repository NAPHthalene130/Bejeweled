#include "AuthNoticeDialog.h"

AuthNoticeDialog::AuthNoticeDialog(const QString& title, const QString& content, int type, QWidget *parent)
    : QDialog(parent), type(type) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(400, 250);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    // Title
    QLabel *titleLabel = new QLabel(title, this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setFamily("Microsoft YaHei");
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // Content
    QLabel *contentLabel = new QLabel(content, this);
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setWordWrap(true);
    QFont contentFont;
    contentFont.setFamily("Microsoft YaHei");
    contentFont.setPointSize(11);
    contentLabel->setFont(contentFont);

    // Button
    QPushButton *btn = new QPushButton("确 定", this);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(40);
    connect(btn, &QPushButton::clicked, this, &QDialog::accept);

    layout->addWidget(titleLabel);
    layout->addWidget(contentLabel);
    layout->addStretch();
    layout->addWidget(btn);

    // Colors based on type
    QString mainColor;
    if (type == 1) mainColor = "#4CAF50"; // Green
    else if (type == 2) mainColor = "#FFC107"; // Amber
    else mainColor = "#F44336"; // Red

    titleLabel->setStyleSheet("color: " + mainColor + ";");
    
    // Button Style
    QString btnStyle = QString(
        "QPushButton { "
        "background-color: %1; "
        "color: white; "
        "border-radius: 20px; "
        "font-family: 'Microsoft YaHei'; "
        "font-weight: bold; "
        "font-size: 14px; "
        "border: none;"
        "} "
        "QPushButton:hover { opacity: 0.8; }"
        "QPushButton:pressed { background-color: %2; }"
    ).arg(mainColor).arg(mainColor); 
    
    btn->setStyleSheet(btnStyle);

    // Drop Shadow for the dialog
    // Since we are painting on the widget itself, we might need a container or parent for shadow if we want it outside.
    // However, for simplicity in frameless, we often use a container widget inside the dialog.
    // But here, I'll just keep it clean. 

    // Play sound
    QApplication::beep();
}

void AuthNoticeDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    QPainterPath path;
    path.addRoundedRect(rect(), 20, 20);
    
    painter.fillPath(path, Qt::white);
    
    // Border
    QPen pen(QColor("#E0E0E0"));
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
}
