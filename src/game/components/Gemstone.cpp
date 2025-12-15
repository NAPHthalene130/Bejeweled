#include "Gemstone.h"
#include <QPainter>
#include <QPaintEvent>
#include <QPolygon>
#include <QPoint>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QPainterPath>

Gemstone::Gemstone(int type, std::string style, QWidget* parent) 
    : QPushButton(parent), type(type), style(style) {
    setFixedSize(100, 100);
}

Gemstone::~Gemstone() {
}

int Gemstone::getType() const {
    return type;
}

void Gemstone::setType(int type) {
    if (this->type != type) {
        this->type = type;
        update();
    }
}

std::string Gemstone::getStyle() const {
    return style;
}

void Gemstone::setStyle(const std::string& style) {
    if (this->style != style) {
        this->style = style;
        update();
    }
}

void Gemstone::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw button background/border if needed, or just custom shape
    // Since we are inheriting QPushButton, we might want to let base class draw?
    // But user asked for custom shapes. Let's draw custom shapes.
    // If user wants button behavior (hover/press), we can handle that.
    
    // Optional: Draw standard button background if style requires it
    // QPushButton::paintEvent(event); 

    if (style == "XXXXX") {
        // Placeholder for specific style
        painter.fillRect(rect(), Qt::black);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "X");
    } else {
        // Default style: 8 types of 2D patterns
        drawDefaultStyle(painter);
    }
}

void Gemstone::drawDefaultStyle(QPainter& painter) {
    // Common setup
    QRect r = rect().adjusted(10, 10, -10, -10); // Padding
    int cx = width() / 2;
    int cy = height() / 2;

    QColor color;
    
    // Define colors for 8 types
    switch (type % 8) {
        case 0: color = QColor(255, 50, 50); break;   // Red
        case 1: color = QColor(50, 255, 50); break;   // Green
        case 2: color = QColor(50, 50, 255); break;   // Blue
        case 3: color = QColor(255, 255, 50); break;  // Yellow
        case 4: color = QColor(255, 50, 255); break;  // Magenta
        case 5: color = QColor(50, 255, 255); break;  // Cyan
        case 6: color = QColor(255, 150, 50); break;  // Orange
        case 7: color = QColor(200, 200, 200); break; // Grey/White
        default: color = Qt::white; break;
    }

    // Hover effect
    if (underMouse()) {
        color = color.lighter(120);
    }
    if (isDown()) {
        color = color.darker(120);
    }

    painter.setBrush(QBrush(color));
    painter.setPen(QPen(Qt::white, 2));

    switch (type % 8) {
        case 0: // Circle
            painter.drawEllipse(r);
            break;
        
        case 1: // Square (Rounded)
            painter.drawRoundedRect(r, 10, 10);
            break;
        
        case 2: // Triangle
        {
            QPolygon triangle;
            triangle << QPoint(cx, r.top()) << QPoint(r.right(), r.bottom()) << QPoint(r.left(), r.bottom());
            painter.drawPolygon(triangle);
            break;
        }

        case 3: // Diamond
        {
            QPolygon diamond;
            diamond << QPoint(cx, r.top()) << QPoint(r.right(), cy) << QPoint(cx, r.bottom()) << QPoint(r.left(), cy);
            painter.drawPolygon(diamond);
            break;
        }

        case 4: // Hexagon
        {
            QPolygon hexagon;
            int w = r.width();
            int h = r.height();
            hexagon << QPoint(cx - w/4, r.top()) << QPoint(cx + w/4, r.top())
                    << QPoint(r.right(), cy) << QPoint(cx + w/4, r.bottom())
                    << QPoint(cx - w/4, r.bottom()) << QPoint(r.left(), cy);
            painter.drawPolygon(hexagon);
            break;
        }

        case 5: // Star (4 points / concave diamond)
        {
            QPolygon star;
            star << QPoint(cx, r.top()) << QPoint(cx + 10, cy - 10) 
                    << QPoint(r.right(), cy) << QPoint(cx + 10, cy + 10)
                    << QPoint(cx, r.bottom()) << QPoint(cx - 10, cy + 10)
                    << QPoint(r.left(), cy) << QPoint(cx - 10, cy - 10);
            painter.drawPolygon(star);
            break;
        }

        case 6: // Cross
        {
            QPainterPath path;
            int thick = 20;
            path.addRect(cx - thick/2, r.top(), thick, r.height());
            path.addRect(r.left(), cy - thick/2, r.width(), thick);
            painter.drawPath(path.simplified());
            break;
        }

        case 7: // Ring / Donut
        {
            painter.drawEllipse(r);
            painter.setBrush(QBrush(QColor(30, 30, 30))); 
            painter.drawEllipse(r.adjusted(20, 20, -20, -20));
            break;
        }
    }
    
    // Add a "shine" effect
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 100));
    painter.drawEllipse(cx - 20, cy - 20, 20, 15);
}
