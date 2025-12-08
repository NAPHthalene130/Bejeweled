#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
};

#endif // LOGINWINDOW_H
