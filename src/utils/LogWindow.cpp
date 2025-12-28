#include "LogWindow.h"
#include <QDateTime>

LogWindow::LogWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("Log Window");
    resize(800, 600); // Increased size

    // Apply grey background and white text style
    setStyleSheet("background-color: #333333; color: #FFFFFF;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    // Ensure textEdit inherits or has compatible style
    textEdit->setStyleSheet("background-color: #222222; color: #FFFFFF; border: 1px solid #555555;");
    layout->addWidget(textEdit);

    clearButton = new QPushButton("清空", this);
    clearButton->setStyleSheet("QPushButton { background-color: #555555; color: white; border: none; padding: 5px; } QPushButton:hover { background-color: #666666; }");
    layout->addWidget(clearButton);

    connect(clearButton, &QPushButton::clicked, this, &LogWindow::clearLog);
}

void LogWindow::clearLog() {
    textEdit->clear();
}

void LogWindow::logWrite(std::string str) {
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString logMsg = QString("[%1] %2").arg(timeStr).arg(QString::fromStdString(str));
    textEdit->append(logMsg);
}
