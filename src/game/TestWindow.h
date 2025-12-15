#ifndef TEST_WINDOW_H
#define TEST_WINDOW_H

#include <QWidget>
#include <vector>

class Gemstone;

class TestWindow : public QWidget {
    Q_OBJECT
public:
    explicit TestWindow(QWidget* parent = nullptr);
    ~TestWindow();

private:
    std::vector<Gemstone*> gemstones;
};

#endif // TEST_WINDOW_H
