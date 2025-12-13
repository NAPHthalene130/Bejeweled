#ifndef AUTHNOTICEDIALOG_H
#define AUTHNOTICEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>

class AuthNoticeDialog : public QDialog {
    Q_OBJECT
public:
    // type: 1-Safe(Info), 2-Warning, 3-Error
    explicit AuthNoticeDialog(const QString& title, const QString& content, int type, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int type;
};

#endif // AUTHNOTICEDIALOG_H
