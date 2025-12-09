#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QWidget>

class AuthWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();
};

#endif // AUTHWINDOW_H
