#include "SingleModeGameWidget.h"
#include "../components/Gemstone.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <Qt3DExtras/Qt3DWindow>

SingleModeGameWidget::SingleModeGameWidget(QWidget* parent, GameWindow* gameWindow) 
    : QWidget(parent), gameWindow(gameWindow), canOpe(true), nowTimeHave(0), mode(1) {
    
    // Initialize Timer
    timer = new QTimer(this);
    
    // Initialize 3D Window
    game3dWindow = new Qt3DExtras::Qt3DWindow();
    
    // Create container for 3D window
    container3d = QWidget::createWindowContainer(game3dWindow);
    container3d->setFixedSize(720, 720); 
    
    // Layout - Left Center
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(50, 0, 50, 0); // Add some margin
    
    // Create a vertical layout to center the 3D container vertically if needed, 
    // or just add it to HBox. QHBoxLayout aligns items vertically center by default usually, 
    // but let's be explicit if needed.
    
    mainLayout->addWidget(container3d);
    mainLayout->addStretch(1); // Push content to the left
    
    setLayout(mainLayout);
}

SingleModeGameWidget::~SingleModeGameWidget() {
    if (game3dWindow) {
        delete game3dWindow;
    }
}

Qt3DExtras::Qt3DWindow* SingleModeGameWidget::getGame3dWindow() const {
    return game3dWindow;
}

std::vector<std::vector<Gemstone*>> SingleModeGameWidget::getGemstoneContainer() const {
    return gemstoneContainer;
}

void SingleModeGameWidget::setGemstoneContainer(const std::vector<std::vector<Gemstone*>>& container) {
    this->gemstoneContainer = container;
}

std::string SingleModeGameWidget::getStyle() const {
    return style;
}

void SingleModeGameWidget::setStyle(const std::string& style) {
    this->style = style;
}

bool SingleModeGameWidget::getCanOpe() const {
    return canOpe;
}

void SingleModeGameWidget::setCanOpe(bool canOpe) {
    this->canOpe = canOpe;
}

QTimer* SingleModeGameWidget::getTimer() const {
    return timer;
}

void SingleModeGameWidget::setTimer(QTimer* timer) {
    this->timer = timer;
}

int SingleModeGameWidget::getNowTimeHave() const {
    return nowTimeHave;
}

void SingleModeGameWidget::setNowTimeHave(int time) {
    this->nowTimeHave = time;
}

int SingleModeGameWidget::getMode() const {
    return mode;
}

void SingleModeGameWidget::setMode(int mode) {
    this->mode = mode;
}

void SingleModeGameWidget::reset(int mode) {
    this->mode = mode;
    this->canOpe = true;
    this->nowTimeHave = 0;
    
    // Clear existing gemstones if any
    for (auto& row : gemstoneContainer) {
        for (auto* gem : row) {
            if (gem) {
                gem->deleteLater();
            }
        }
    }
    gemstoneContainer.clear();
    
    // Reset timer
    if (timer->isActive()) {
        timer->stop();
    }
    
    // Additional reset logic can be added here
}
