#ifndef LOG_WINDOW_H
#define LOG_WINDOW_H

#include <QWidget>
#include <QTextEdit>
#include <string>
#include <QVBoxLayout>

#include <QPushButton>

class LogWindow : public QWidget {
    Q_OBJECT
public:
    explicit LogWindow(QWidget* parent = nullptr);
    void logWrite(std::string str);

private slots:
    void clearLog();

private:
    QTextEdit* textEdit;
    QPushButton* clearButton;
};

#endif // LOG_WINDOW_H
