#include "widgets/AchievementsWidget.h"
#include "components/AchievementItemWidget.h"
#include "game/data/AchievementData.h"
#include <QDebug>
#include <QLayoutItem>
#include <QSpacerItem>

// --- 模拟 GameWindow 接口实现（用于编译调试，请在您的实际 GameWindow 中实现这些代码）---
const std::vector<AchievementData>& GameWindow::getAchievementsContainer() const {
    static std::vector<AchievementData> dummyAchievements = {
        {"First Gem Swap", "Successfully swap two gems.", true},
        {"Triple Match", "Achieve a match of 3 gems.", true},
        {"Quadruple Power", "Achieve a match of 4 gems.", true},
        {"Five-in-a-Row", "Achieve a match of 5 gems.", true},
        {"Speedster I", "Clear Level 1 in under 60 seconds.", true},
        {"Puzzle Master", "Solve 5 challenges.", false},
        {"High Scorer", "Reach a score of 10,000.", false},
        {"Chain Reaction", "Get 3 cascades in one turn.", false},
        {"Level 10 Cleared", "Complete the 10th level.", false},
        {"Gem Collector", "Collect 1000 blue gems.", false},
        {"Novice", "Play the game for 1 hour.", false},
        {"Expert Swapper", "Perform 500 total swaps.", false},
        {"Mega Combo", "Create a 6+ match combo.", false},
        {"Hidden Star", "Find the secret item.", false},
        {"Marathoner", "Play for 5 consecutive hours.", false}
    };
    return dummyAchievements;
}
// ---------------------------------------------------------------------------------------------------


AchievementsWidget::AchievementsWidget(GameWindow* gameWindow, QWidget* parent)
    : QWidget(parent), m_gameWindow(gameWindow)
{
    setStyleSheet("QWidget { background-color: #1A1A1A; color: white; }");
    
    setupUI();
    updateAchievementsWidget(); 
}

void AchievementsWidget::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    QLabel* title = new QLabel("💎 Achievements List");
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(title);
    m_mainLayout->addSpacing(20);

    m_achievementsGrid = new QGridLayout();
    m_achievementsGrid->setSpacing(15);
    m_mainLayout->addLayout(m_achievementsGrid);
    
    m_mainLayout->addStretch(1);

    QHBoxLayout* paginationLayout = new QHBoxLayout();
    
    m_prevButton = new QPushButton("← Previous");
    m_pageInfoLabel = new QLabel("Page 0 / 0");
    m_nextButton = new QPushButton("Next →");

    QString buttonStyle = "QPushButton { padding: 8px 15px; border-radius: 5px; background-color: #4A4A4A; color: white; }"
                          "QPushButton:disabled { background-color: #303030; color: #888888; }";
    m_prevButton->setStyleSheet(buttonStyle);
    m_nextButton->setStyleSheet(buttonStyle);

    m_pageInfoLabel->setAlignment(Qt::AlignCenter);
    m_pageInfoLabel->setFixedWidth(120);

    paginationLayout->addStretch(1);
    paginationLayout->addWidget(m_prevButton);
    paginationLayout->addWidget(m_pageInfoLabel);
    paginationLayout->addWidget(m_nextButton);
    paginationLayout->addStretch(1);
    
    m_mainLayout->addLayout(paginationLayout);

    connect(m_prevButton, &QPushButton::clicked, this, &AchievementsWidget::goToPreviousPage);
    connect(m_nextButton, &QPushButton::clicked, this, &AchievementsWidget::goToNextPage);
}

void AchievementsWidget::calculateTotalPages() {
    int totalAchievements = m_currentAchievements.size();
    if (totalAchievements == 0) {
        m_totalPages = 0;
    } else {
        m_totalPages = (totalAchievements + ACHIEVEMENTS_PER_PAGE - 1) / ACHIEVEMENTS_PER_PAGE;
    }
}

void AchievementsWidget::loadPage(int pageIndex) {
    QLayoutItem* item;
    while ((item = m_achievementsGrid->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (m_currentAchievements.empty()) {
        m_pageInfoLabel->setText("No Achievements");
        m_prevButton->setEnabled(false);
        m_nextButton->setEnabled(false);
        return;
    }

    int startIndex = pageIndex * ACHIEVEMENTS_PER_PAGE;
    int endIndex = qMin(startIndex + ACHIEVEMENTS_PER_PAGE, (int)m_currentAchievements.size());

    int row = 0;
    int col = 0;
    const int COLUMNS = 2;

    for (int i = startIndex; i < endIndex; ++i) {
        const auto& data = m_currentAchievements[i];
        
        AchievementItemWidget* itemWidget = new AchievementItemWidget(data, this);
        m_achievementsGrid->addWidget(itemWidget, row, col);

        col++;
        if (col >= COLUMNS) {
            col = 0;
            row++;
        }
    }
    
    for (int r = row; r < ACHIEVEMENTS_PER_PAGE / COLUMNS; ++r) {
        for (int c = col; c < COLUMNS; ++c) {
             QLayoutItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
             m_achievementsGrid->addItem(spacer, r, c);
        }
        col = 0;
    }

    m_pageInfoLabel->setText(QString("Page %1 / %2")
                                 .arg(pageIndex + 1)
                                 .arg(m_totalPages));
    
    m_prevButton->setEnabled(pageIndex > 0);
    m_nextButton->setEnabled(pageIndex < m_totalPages - 1);
}

void AchievementsWidget::updateAchievementsWidget() {
    m_currentAchievements = m_gameWindow->getAchievementsContainer(); 
    
    calculateTotalPages(); 
    
    if (m_currentPage >= m_totalPages) {
        m_currentPage = (m_totalPages > 0) ? m_totalPages - 1 : 0;
    }
    m_currentPage = qMax(0, m_currentPage);

    loadPage(m_currentPage); 
}

void AchievementsWidget::goToNextPage() {
    if (m_currentPage < m_totalPages - 1) {
        m_currentPage++;
        loadPage(m_currentPage);
    }
}

void AchievementsWidget::goToPreviousPage() {
    if (m_currentPage > 0) {
        m_currentPage--;
        loadPage(m_currentPage);
    }
}