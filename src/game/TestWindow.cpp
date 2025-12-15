#include "TestWindow.h"
#include "components/Gemstone.h"
#include <QGridLayout>
#include <QLabel>

TestWindow::TestWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("Gemstone Styles Test");
    resize(600, 400);

    QGridLayout* layout = new QGridLayout(this);

    // Loop to create 8 types of gemstones
    for (int i = 0; i < 8; ++i) {
        // Create Gemstone
        Gemstone* gem = new Gemstone(i, "default");
        gemstones.push_back(gem);

        // Add to layout (2 rows of 4)
        int row = i / 4;
        int col = i % 4;

        // Add a label for the type
        QLabel* label = new QLabel("Type " + QString::number(i), this);
        label->setAlignment(Qt::AlignCenter);
        
        QVBoxLayout* cellLayout = new QVBoxLayout();
        cellLayout->addWidget(gem);
        cellLayout->addWidget(label);
        cellLayout->setAlignment(Qt::AlignCenter);

        layout->addLayout(cellLayout, row, col);
    }
}

TestWindow::~TestWindow() {
    for (Gemstone* gem : gemstones) {
        delete gem;
    }
    gemstones.clear();
}
