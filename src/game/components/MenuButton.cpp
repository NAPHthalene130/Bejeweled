#include "MenuButton.h"
#include "../managers/AudioManager.h"
#include <QFont>
#include <QEnterEvent>

MenuButton::MenuButton(int width, int height, int fontSize, const QColor& fontColor, const QString& text, QWidget* parent) 
    : QPushButton(text, parent) {
    setFixedSize(width, height);

    QFont font = this->font();
    font.setPointSize(fontSize);
    font.setBold(true);
    font.setFamily("Segoe UI"); // Use a clean modern font
    setFont(font);

    // Color string for the accent color
    QString accentColor = fontColor.name(QColor::HexArgb);
    // Semi-transparent version of accent for background? No, let's stick to dark theme.
    
    // Style: Sci-Fi / Modern Look
    // Base: Dark semi-transparent background
    // Border: Accent color
    // Text: Accent color (or white)
    // Hover: Fill with accent color, text turns dark
    
    QString style = QString(
        "MenuButton {"
        "   background-color: rgba(20, 30, 50, 180);"  // Dark blue-grey semi-transparent
        "   color: white;"                             // White text by default
        "   border: 2px solid %1;"                     // Colored border
        "   border-radius: 0px;"                       // No rounded corners
        "   padding: 4px;"
        "}"
        "MenuButton:hover {"
        "   background-color: %1;"                     // Fill with color on hover
        "   color: black;"                             // Text turns black for contrast
        "   border: 2px solid rgba(255, 255, 255, 200);" // White-ish border
        "}"
        "MenuButton:pressed {"
        "   background-color: white;"
        "   color: black;"
        "   border: 2px solid %1;"
        "}"
    ).arg(accentColor);

    setStyleSheet(style);
    
    // Optional: Add a shadow effect if we wanted (requires QGraphicsDropShadowEffect), 
    // but CSS is cleaner for now.

    connect(this, &QPushButton::clicked, this, []() {
        AudioManager::instance().playClickSound();
    });
}

MenuButton::~MenuButton() {
}

void MenuButton::enterEvent(QEnterEvent* event) {
    AudioManager::instance().playHoverSound();
    QPushButton::enterEvent(event);
}
