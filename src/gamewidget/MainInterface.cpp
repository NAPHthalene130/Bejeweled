#include "MainInterface.h"
#include <QLabel>
#include <QVBoxLayout>

MainInterface::MainInterface(QWidget *parent) : QWidget(parent) {
    setWindowTitle("主界面");
    resize(1600, 1000);
    
    // 空界面占位内容
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("欢迎使用系统", this), 0, Qt::AlignCenter);
}