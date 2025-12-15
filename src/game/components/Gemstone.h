#ifndef GEMSTONE_H
#define GEMSTONE_H

#include <string>
#include <QPushButton>
#include <QPainter>

class Gemstone : public QPushButton {
    Q_OBJECT
public:
    Gemstone(int type, std::string style, QWidget* parent = nullptr);
    ~Gemstone();

    int getType() const;
    void setType(int type);

    std::string getStyle() const;
    void setStyle(const std::string& style);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int type;
    std::string style;

    void drawDefaultStyle(QPainter& painter);
};

#endif // GEMSTONE_H
