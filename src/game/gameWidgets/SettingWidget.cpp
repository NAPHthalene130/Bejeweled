#include "SettingWidget.h"
#include "../GameWindow.h"
#include "../../utils/BackgroundManager.h"
#include "../../utils/ResourceUtils.h"
#include "../../game/components/Gemstone.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStyleOption>
#include <QPainter>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QPropertyAnimation>
#include <random>
#include <cmath>
#include <QGraphicsDropShadowEffect>
#include "MenuWidget.h"
#include "../../utils/BGMManager.h"
#include <QSettings>

// ==================== 自定义保存成功对话框 ====================
class SettingSaveDialog : public QDialog {
public:
    explicit SettingSaveDialog(const QString& message, QWidget* parent = nullptr) 
        : QDialog(parent), messageText(message) {
        setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        setModal(true);
        setFixedSize(480, 280);
        setWindowTitle("提示");

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(30, 28, 30, 26);
        layout->setSpacing(20);

        // 设置对话框背景
        setStyleSheet(R"(
            QDialog {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 rgb(50, 35, 25),
                    stop:0.5 rgb(40, 28, 20),
                    stop:1 rgb(35, 25, 18));
                border: 2px solid rgb(255, 160, 60);
                border-radius: 20px;
            }
        )");

        // 成功图标（使用Unicode符号）
        auto* iconLabel = new QLabel("✓", this);
        QFont iconFont = iconLabel->font();
        iconFont.setPointSize(48);
        iconFont.setBold(true);
        iconFont.setFamily("Microsoft YaHei");
        iconLabel->setFont(iconFont);
        iconLabel->setAlignment(Qt::AlignHCenter);
        iconLabel->setStyleSheet(R"(
            color: #FFD700;
            background: transparent;
        )");
        iconLabel->setFixedHeight(60);

        // 标题标签
        auto* titleLabel = new QLabel("保存成功", this);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(18);
        titleFont.setBold(true);
        titleFont.setFamily("Microsoft YaHei");
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignHCenter);
        titleLabel->setStyleSheet("color: rgba(255,255,255,245); background: transparent;");

        // 消息内容标签
        auto* contentLabel = new QLabel(message, this);
        QFont contentFont = contentLabel->font();
        contentFont.setPointSize(13);
        contentFont.setFamily("Microsoft YaHei");
        contentLabel->setFont(contentFont);
        contentLabel->setAlignment(Qt::AlignHCenter);
        contentLabel->setWordWrap(true);
        contentLabel->setStyleSheet("color: rgba(255,255,255,220); background: transparent;");

        // 确定按钮
        auto* okBtn = new QPushButton("确定", this);
        okBtn->setFixedSize(160, 48);
        okBtn->setCursor(Qt::PointingHandCursor);
        okBtn->setStyleSheet(R"(
            QPushButton {
                color: white;
                border-radius: 14px;
                border: 2px solid rgba(255,215,0,100);
                background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                    stop:0 rgba(255,160,60,230), 
                    stop:1 rgba(255,140,40,230));
                font-family: 'Microsoft YaHei';
                font-size: 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                    stop:0 rgba(255,180,80,250), 
                    stop:1 rgba(255,160,60,250));
                border: 2px solid rgba(255,215,0,150);
            }
            QPushButton:pressed {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                    stop:0 rgba(230,140,50,230), 
                    stop:1 rgba(210,120,30,230));
            }
        )");

        connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);

        layout->addWidget(iconLabel);
        layout->addWidget(titleLabel);
        layout->addWidget(contentLabel);
        layout->addSpacing(10);
        
        QHBoxLayout* btnLayout = new QHBoxLayout();
        btnLayout->addStretch();
        btnLayout->addWidget(okBtn);
        btnLayout->addStretch();
        layout->addLayout(btnLayout);
    }

private:
    QString messageText;
};

// ==================== 静态方法 ====================

int SettingWidget::getBackgroundMusicVolume() {
    QSettings settings("GemMatch", "Settings");
    return settings.value("Music/BgVolume", 50).toInt();
}

int SettingWidget::getEliminateSoundVolume() {
    QSettings settings("GemMatch", "Settings");
    return settings.value("Music/EliminateVolume", 50).toInt();
}

bool SettingWidget::isBackgroundMusicEnabled() {
    QSettings settings("GemMatch", "Settings");
    return settings.value("Music/BgEnable", true).toBool();
}

bool SettingWidget::isEliminateSoundEnabled() {
    QSettings settings("GemMatch", "Settings");
    return settings.value("Music/EliminateEnable", true).toBool();
}

QString SettingWidget::getMenuBackgroundImage() {
    QSettings settings("GemMatch", "Settings");
    QString defaultBg = QString::fromStdString(ResourceUtils::getPath("images/default_bg.png"));
    return settings.value("Image/MenuBg", defaultBg).toString();
}

QString SettingWidget::getGemStyle() {
    QSettings settings("GemMatch", "Settings");
    return settings.value("Game/GemStyle", "几何体").toString();
}

// ==================== 构造函数 ====================

SettingWidget::SettingWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow), animTime(0) {
    settings = new QSettings("GemMatch", "Settings");
    setWindowTitle("设置");

    setMinimumSize(800, 600);
    resize(1600, 1000);


    // ====================== 1. 初始化动画 ======================
    initParticles();
    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &SettingWidget::updateAnimation);
    animTimer->start(16);

    // ====================== 2. 初始化控件 ======================
    
    // 音乐设置控件
    bgMusicSlider = new QSlider(Qt::Horizontal, this);
    eliminateSoundSlider = new QSlider(Qt::Horizontal, this);
    bgMusicEnableBox = new QCheckBox("启用", this);
    eliminateSoundEnableBox = new QCheckBox("启用", this);
    QLabel* bgMusicLabel = new QLabel("背景音乐", this);
    QLabel* eliminateSoundLabel = new QLabel("消除音效", this);
    bgVolLabel = new QLabel("50%", this);
    eliminateVolLabel = new QLabel("50%", this);
    eliminateSoundSelectLabel = new QLabel("音效类型", this);
    eliminateSoundCombo = new QComboBox(this);

    // 图像设置控件
    resolutionCombo = new QComboBox(this);
    selectBgBtn = new QPushButton("选择图片", this);
    bgPreviewLabel = new QLabel(this);
    QLabel* resolutionLabel = new QLabel("分辨率", this);
    QLabel* bgLabel = new QLabel("菜单背景图", this);

    // 游戏设置控件
    switchInterfaceBtn = new QPushButton("切换备用界面（预留）", this);
    QLabel* gameTipLabel = new QLabel("游戏功能设置", this);
    QLabel* gemStyleLabel = new QLabel("宝石风格", this);
    gemStyleCombo = new QComboBox(this);
    gemStyleDescLabel = new QLabel("", this);

    difficultyLabel = new QLabel("难度", this);
    difficultyCombo = new QComboBox(this);
    difficultyCombo->addItems({"简单", "中等", "困难"});

    // 按钮
    saveBtn = new QPushButton("保存设置", this);
    backBtn = new QPushButton("返回菜单", this);

    // 标签页
    tabWidget = new QTabWidget(this);

    // ====================== 3. 样式设置 ======================
    
    // 标签样式
    QList<QLabel*> labels = {bgMusicLabel, eliminateSoundLabel, bgVolLabel, eliminateVolLabel,
                             resolutionLabel, bgLabel, gameTipLabel, gemStyleLabel, 
                             eliminateSoundSelectLabel, gemStyleDescLabel, difficultyLabel};
    for (QLabel* label : labels) {
        label->setStyleSheet(R"(
            color: #FFF5E6;
            font-size: 18px;
            font-weight: 500;
            text-shadow: 0 0 8px rgba(255, 160, 60, 0.5);
        )");
        label->setFixedHeight(28);
    }
    bgVolLabel->setFixedWidth(56);
    bgVolLabel->setAlignment(Qt::AlignCenter);
    eliminateVolLabel->setFixedWidth(56);
    eliminateVolLabel->setAlignment(Qt::AlignCenter);
    
    // 风格描述标签特殊样式
    gemStyleDescLabel->setStyleSheet(R"(
        color: #FFD700;
        font-size: 14px;
        font-style: italic;
        text-shadow: 0 0 8px rgba(255, 215, 0, 0.5);
    )");
    gemStyleDescLabel->setFixedHeight(24);
    gemStyleDescLabel->setWordWrap(true);

    // 复选框样式
    QString checkBoxStyle = R"(
        QCheckBox {
            color: #FFF5E6;
            font-size: 18px;
            spacing: 10px;
        }
        QCheckBox::indicator {
            width: 24px;
            height: 24px;
            border-radius: 12px;
            background: rgba(255, 245, 230, 0.2);
            border: 2px solid #FFA54F;
        }
        QCheckBox::indicator:checked {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FFA54F, stop:1 #FF8C00);
            border: 2px solid #FF7F24;
        }
    )";
    bgMusicEnableBox->setStyleSheet(checkBoxStyle);
    eliminateSoundEnableBox->setStyleSheet(checkBoxStyle);
    bgMusicEnableBox->setFixedHeight(28);
    eliminateSoundEnableBox->setFixedHeight(28);

    // 滑块样式
    QString sliderStyle = R"(
        QSlider::groove:horizontal {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0, 
                                       stop:0 rgba(255, 160, 60, 0.2), 
                                       stop:1 rgba(255, 180, 80, 0.3));
            height: 10px;
            border-radius: 5px;
        }
        QSlider::handle:horizontal {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FFA54F, stop:1 #FF8C00);
            border: 2px solid #FFF5E6;
            width: 28px;
            height: 28px;
            margin: -9px 0;
            border-radius: 14px;
        }
        QSlider::handle:horizontal:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FFC14F, stop:1 #FFA500);
        }
        QSlider:disabled::groove:horizontal {
            background: rgba(255, 160, 60, 0.1);
        }
        QSlider:disabled::handle:horizontal {
            background: rgba(255, 160, 60, 0.5);
            border: 2px solid rgba(255, 245, 230, 0.5);
        }
    )";
    bgMusicSlider->setStyleSheet(sliderStyle);
    eliminateSoundSlider->setStyleSheet(sliderStyle);
    bgMusicSlider->setRange(0, 100);
    eliminateSoundSlider->setRange(0, 100);
    bgMusicSlider->setFixedHeight(28);
    eliminateSoundSlider->setFixedHeight(28);
    bgMusicSlider->setMinimumWidth(550);
    eliminateSoundSlider->setMinimumWidth(550);

    // 下拉框样式
    QString comboStyle = R"(
        QComboBox {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 rgba(255, 160, 60, 0.15), 
                                       stop:1 rgba(255, 140, 40, 0.25));
            color: #FFF5E6;
            border: 2px solid #FFF5E6;
            border-radius: 6px;
            padding: 6px 12px;
            font-size: 16px;
            min-width: 200px;
            height: 40px;
        }
        QComboBox::drop-down {
            border-left: 2px solid #FFF5E6;
            border-top-right-radius: 4px;
            border-bottom-right-radius: 4px;
            width: 40px;
        }
        QComboBox QAbstractItemView {
            background: rgba(60, 40, 20, 0.95);
            color: #FFF5E6;
            selection-background-color: #FF8C00;
            border: 2px solid #FFA54F;
            border-radius: 4px;
        }
        QComboBox::item {
            padding: 8px 12px;
            min-height: 32px;
        }
        QComboBox::item:selected {
            background: #FF8C00;
        }
    )";
    resolutionCombo->setStyleSheet(comboStyle);
    gemStyleCombo->setStyleSheet(comboStyle);
    eliminateSoundCombo->setStyleSheet(comboStyle);
    difficultyCombo->setStyleSheet(comboStyle);
    
    resolutionCombo->addItems({
        "1280x720", 
        "1366x768", 
        "1440x900", 
        "1600x900", 
        "1600x1000", 
        "1920x1080", 
        "1920x1200", 
        "2560x1440"
    });
    eliminateSoundCombo->addItems({"Manbo", "Original"});
    
    // 更新宝石风格下拉框选项
    updateGemStyleComboItems();

    // 按钮样式
    QString btnStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FF8C00, stop:1 #FF7F24);
            color: #FFF5E6;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 16px;
            font-weight: 500;
            height: 44px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FFA54F, stop:1 #FF8C00);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FF7F24, stop:1 #FF6347);
        }
    )";
    selectBgBtn->setStyleSheet(btnStyle);
    switchInterfaceBtn->setStyleSheet(btnStyle);

    saveBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FFA54F, stop:1 #FF8C00);
            color: #FFF5E6;
            border: none;
            border-radius: 6px;
            padding: 12px 0;
            font-size: 18px;
            font-weight: bold;
            height: 44px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FFC14F, stop:1 #FFA54F);
        }
    )");
    saveBtn->setFixedSize(200, 44);

    backBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #87CEEB, stop:1 #4682B4);
            color: #FFF5E6;
            border: none;
            padding: 12px 0;
            font-size: 18px;
            border-radius: 6px;
            height: 44px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #B0E0E6, stop:1 #5F9EA0);
        }
    )");
    backBtn->setFixedSize(200, 44);

    bgPreviewLabel->setFixedSize(160, 96);
    bgPreviewLabel->setStyleSheet(R"(
        border: 2px solid #FFA54F;
        border-radius: 4px;
        background: rgba(255, 245, 230, 0.1);
    )");

    tabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: 2px solid #FFF5E6;
            border-radius: 8px;
            background: rgba(255, 160, 60, 0.1);
            padding: 12px;
        }
        QTabBar::tab {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1, 
                                       stop:0 rgba(255, 160, 60, 0.3), 
                                       stop:1 rgba(255, 140, 40, 0.5));
            color: #FFF5E6;
            padding: 8px 16px;
            margin-right: 5px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            border: 1px solid #FFF5E6;
            border-bottom: none;
            font-size: 16px;
            height: 44px;
        }
        QTabBar::tab:selected {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1, 
                                       stop:0 #FF8C00, stop:1 #FF7F24);
            font-weight: 500;
        }
    )");
    tabWidget->setMinimumHeight(500);

    // ====================== 4. 构建布局 ======================
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(60, 20, 60, 20);
    mainLayout->setSpacing(20);

    // 音乐设置标签页
    QWidget* musicWidget = new QWidget();
    QVBoxLayout* musicLayout = new QVBoxLayout(musicWidget);
    musicLayout->setSpacing(18);
    
    QHBoxLayout* bgMusicLayout = new QHBoxLayout();
    bgMusicLayout->addWidget(bgMusicLabel);
    bgMusicLayout->addSpacing(20);
    bgMusicLayout->addWidget(bgMusicEnableBox);
    bgMusicLayout->addSpacing(20);
    bgMusicLayout->addWidget(bgMusicSlider);
    bgMusicLayout->addSpacing(20);
    bgMusicLayout->addWidget(bgVolLabel);
    
    QHBoxLayout* eliminateSoundLayout = new QHBoxLayout();
    eliminateSoundLayout->addWidget(eliminateSoundLabel);
    eliminateSoundLayout->addSpacing(20);
    eliminateSoundLayout->addWidget(eliminateSoundEnableBox);
    eliminateSoundLayout->addSpacing(20);
    eliminateSoundLayout->addWidget(eliminateSoundSlider);
    eliminateSoundLayout->addSpacing(20);
    eliminateSoundLayout->addWidget(eliminateVolLabel);
    
    QHBoxLayout* eliminateSoundSelectLayout = new QHBoxLayout();
    eliminateSoundSelectLayout->addWidget(eliminateSoundSelectLabel);
    eliminateSoundSelectLayout->addSpacing(20);
    eliminateSoundSelectLayout->addWidget(eliminateSoundCombo);
    eliminateSoundSelectLayout->addStretch();
    
    musicLayout->addLayout(bgMusicLayout);
    musicLayout->addLayout(eliminateSoundLayout);
    musicLayout->addLayout(eliminateSoundSelectLayout);
    musicLayout->addStretch();

    // 图像设置标签页
    QWidget* imageWidget = new QWidget();
    QVBoxLayout* imageLayout = new QVBoxLayout(imageWidget);
    imageLayout->setSpacing(18);
    
    QHBoxLayout* resolutionLayout = new QHBoxLayout();
    resolutionLayout->addWidget(resolutionLabel);
    resolutionLayout->addSpacing(20);
    resolutionLayout->addWidget(resolutionCombo);
    resolutionLayout->addStretch();
    
    QVBoxLayout* bgLayout = new QVBoxLayout();
    QHBoxLayout* bgCtrlLayout = new QHBoxLayout();
    bgCtrlLayout->addWidget(selectBgBtn);
    bgCtrlLayout->addSpacing(20);
    bgCtrlLayout->addWidget(bgPreviewLabel);
    bgCtrlLayout->addStretch();
    bgLayout->addWidget(bgLabel);
    bgLayout->addLayout(bgCtrlLayout);
    
    imageLayout->addLayout(resolutionLayout);
    imageLayout->addLayout(bgLayout);
    imageLayout->addStretch();

    // 游戏设置标签页
    QWidget* gameWidget = new QWidget();
    QVBoxLayout* gameLayout = new QVBoxLayout(gameWidget);
    gameLayout->setSpacing(18);
    
    gameLayout->addWidget(gameTipLabel);
    
    // 宝石风格选择
    QHBoxLayout* gemStyleLayout = new QHBoxLayout();
    gemStyleLayout->addWidget(gemStyleLabel);
    gemStyleLayout->addSpacing(20);
    gemStyleLayout->addWidget(gemStyleCombo);
    gemStyleLayout->addStretch();
    gameLayout->addLayout(gemStyleLayout);
    
    // 风格描述
    gameLayout->addWidget(gemStyleDescLabel);

    // 难度选择
    QHBoxLayout* difficultyLayout = new QHBoxLayout();
    difficultyLayout->addWidget(difficultyLabel);
    difficultyLayout->addSpacing(20);
    difficultyLayout->addWidget(difficultyCombo);
    difficultyLayout->addStretch();
    gameLayout->addLayout(difficultyLayout);
    
    gameLayout->addSpacing(20);
    gameLayout->addWidget(switchInterfaceBtn);
    gameLayout->addStretch();

    // 添加标签页
    tabWidget->addTab(musicWidget, "音乐");
    tabWidget->addTab(imageWidget, "图像");
    tabWidget->addTab(gameWidget, "游戏");

    // 按钮布局
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(30);
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(backBtn);
    btnLayout->addStretch();

    QWidget* btnWidget = new QWidget(this);
    btnWidget->setLayout(btnLayout);
    btnWidget->setMinimumHeight(80);

    mainLayout->addWidget(tabWidget, 1);
    mainLayout->addWidget(btnWidget);

    // ====================== 5. 信号与槽 ======================
    
    // 音量联动
    connect(bgMusicSlider, &QSlider::valueChanged, [=](int val) {
        bgVolLabel->setText(QString("%1%").arg(val));
    });
    connect(eliminateSoundSlider, &QSlider::valueChanged, [=](int val) {
        eliminateVolLabel->setText(QString("%1%").arg(val));
    });

    // 复选框与滑块联动
    connect(bgMusicEnableBox, &QCheckBox::toggled, [=](bool checked) {
        bgMusicSlider->setEnabled(checked);
        bgVolLabel->setEnabled(checked);
    });
    connect(eliminateSoundEnableBox, &QCheckBox::toggled, [=](bool checked) {
        eliminateSoundSlider->setEnabled(checked);
        eliminateVolLabel->setEnabled(checked);
        eliminateSoundCombo->setEnabled(checked);
    });

    // 宝石风格变化
    connect(gemStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingWidget::onGemStyleChanged);

    // 分辨率变化
    connect(resolutionCombo, &QComboBox::currentTextChanged, [this](const QString& text) {
        QStringList parts = text.split("x");
        if (parts.size() == 2 && this->gameWindow) {
            int w = parts[0].toInt();
            int h = parts[1].toInt();
            if (this->gameWindow->isMaximized()) {
                this->gameWindow->showNormal();
            }
            this->gameWindow->resize(w, h);
        }
    });

    // 难度选择变化
    connect(difficultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        int diff = 6;
        if (index == 0) diff = 4;
        else if (index == 1) diff = 6;
        else if (index == 2) diff = 8;
        
        if (this->gameWindow) {
            this->gameWindow->setDifficulty(diff);
        }
    });

    // 保存按钮
    connect(saveBtn, &QPushButton::clicked, this, &SettingWidget::saveSettings);

    // 返回按钮
    connect(backBtn, &QPushButton::clicked, this, [this]() {
        this->hide();
        if (this->gameWindow) {
            this->gameWindow->switchWidget(this->gameWindow->getMenuWidget());
        }
    });

    // 选择背景图
    connect(selectBgBtn, &QPushButton::clicked, this, &SettingWidget::selectMenuBackground);

    // ====================== 6. 加载设置 ======================
    loadSettings();
}

// ==================== 更新宝石风格下拉框 ====================

void SettingWidget::updateGemStyleComboItems() {
    gemStyleCombo->clear();
    
    // 获取可用风格
    QStringList availableStyles = GemstoneModelManager::instance().getAvailableStyles();
    
    // 如果没有可用风格，至少添加几何体
    if (availableStyles.isEmpty()) {
        availableStyles << "几何体";
    }
    
    // 添加到下拉框
    gemStyleCombo->addItems(availableStyles);
    
    qDebug() << "[SettingWidget] Available gem styles:" << availableStyles;
}

// ==================== 宝石风格变化处理 ====================

void SettingWidget::onGemStyleChanged(int index) {
    Q_UNUSED(index);
    
    QString styleName = gemStyleCombo->currentText();
    
    // 更新描述
    if (styleName == "几何体") {
        gemStyleDescLabel->setText("使用基础几何形状：球体、立方体、圆锥等");
    } else if (styleName == "宝石") {
        gemStyleDescLabel->setText("精美的宝石模型：水晶、钻石、翡翠等");
    } else if (styleName == "八大行星") {
        gemStyleDescLabel->setText("太阳系行星：水星、金星、地球、火星...");
    } else if (styleName == "美食") {
        gemStyleDescLabel->setText("诱人的美食：薯条、汉堡、披萨、冰淇淋...");
    } else if (styleName == "自定义风格1") {
        gemStyleDescLabel->setText("用户自定义风格1");
    } else if (styleName == "自定义风格2") {
        gemStyleDescLabel->setText("用户自定义风格2");
    } else {
        gemStyleDescLabel->setText("");
    }
    
    qDebug() << "[SettingWidget] Gem style selected:" << styleName;
}

// ==================== 加载设置 ====================

void SettingWidget::loadSettings() {
    if (!bgMusicSlider || !eliminateSoundSlider || !bgMusicEnableBox || !eliminateSoundEnableBox) {
        QMessageBox::critical(this, "错误", "控件未初始化，无法加载设置！");
        return;
    }

    // 音乐设置
    int bgVolume = settings->value("Music/BgVolume", 50).toInt();
    int eliminateVol = settings->value("Music/EliminateVolume", 50).toInt();
    bool bgEnable = settings->value("Music/BgEnable", true).toBool();
    bool eliminateEnable = settings->value("Music/EliminateEnable", true).toBool();
    QString soundType = settings->value("Music/EliminateType", "Manbo").toString();

    bgMusicEnableBox->setChecked(bgEnable);
    eliminateSoundEnableBox->setChecked(eliminateEnable);
    bgMusicSlider->setValue(bgVolume);
    bgMusicSlider->setEnabled(bgEnable);
    eliminateSoundSlider->setValue(eliminateVol);
    eliminateSoundSlider->setEnabled(eliminateEnable);
    eliminateSoundCombo->setCurrentText(soundType);
    eliminateSoundCombo->setEnabled(eliminateEnable);
    bgVolLabel->setText(QString("%1%").arg(bgVolume));
    bgVolLabel->setEnabled(bgEnable);
    eliminateVolLabel->setText(QString("%1%").arg(eliminateVol));
    eliminateVolLabel->setEnabled(eliminateEnable);

    // 加载难度设置
    if (gameWindow) {
        int diff = gameWindow->getDifficulty();
        int index = 1; // 默认中等
        if (diff == 4) index = 0;
        else if (diff == 6) index = 1;
        else if (diff == 8) index = 2;
        difficultyCombo->setCurrentIndex(index);
    }

    // 图像设置
    QString resolution = settings->value("Image/Resolution", "1600x1000").toString();
    QString defaultBg = QString::fromStdString(ResourceUtils::getPath("images/default_bg.png"));
    currentBgPath = settings->value("Image/MenuBg", defaultBg).toString();
    resolutionCombo->setCurrentText(resolution);

    QPixmap bgPixmap(currentBgPath);
    if (!bgPixmap.isNull()) {
        bgPreviewLabel->setPixmap(bgPixmap.scaled(bgPreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // 游戏设置 - 宝石风格
    QString gemStyle = settings->value("Game/GemStyle", "几何体").toString();
    int styleIndex = gemStyleCombo->findText(gemStyle);
    if (styleIndex >= 0) {
        gemStyleCombo->setCurrentIndex(styleIndex);
    } else {
        gemStyleCombo->setCurrentIndex(0);  // 默认几何体
    }
    
    // 触发一次风格变化以更新描述
    onGemStyleChanged(gemStyleCombo->currentIndex());
}

// ==================== 保存设置 ====================

void SettingWidget::saveSettings() {
    if (!bgMusicSlider || !eliminateSoundSlider || !bgMusicEnableBox || !eliminateSoundEnableBox) {
        QMessageBox::critical(this, "错误", "控件未初始化，无法保存设置！");
        return;
    }

    // 保存音乐设置
    settings->setValue("Music/BgVolume", bgMusicSlider->value());
    settings->setValue("Music/EliminateVolume", eliminateSoundSlider->value());
    settings->setValue("Music/BgEnable", bgMusicEnableBox->isChecked());
    settings->setValue("Music/EliminateEnable", eliminateSoundEnableBox->isChecked());
    settings->setValue("Music/EliminateType", eliminateSoundCombo->currentText());

    // 保存图像设置
    settings->setValue("Image/Resolution", resolutionCombo->currentText());
    settings->setValue("Image/MenuBg", currentBgPath);

    // 保存游戏设置 - 宝石风格
    QString selectedStyle = gemStyleCombo->currentText();
    settings->setValue("Game/GemStyle", selectedStyle);
    
    // 应用宝石风格变化
    GemstoneModelManager::instance().setCurrentStyleByName(selectedStyle);
    
    // 发送风格变化信号
    emit gemStyleChanged(selectedStyle);

    // 应用背景音乐设置
    BGMManager::instance().setVolume(bgMusicSlider->value());
    if (bgMusicEnableBox->isChecked()) {
        BGMManager::instance().resume();
    } else {
        BGMManager::instance().pause();
    }

    emit backgroundImageChanged(currentBgPath);
    
    // ========== 使用自定义美观对话框 ==========
    QString message = QString("设置已成功保存！\n\n当前宝石风格：%1").arg(selectedStyle);
    SettingSaveDialog dlg(message, this);
    dlg.exec();
}

// ==================== 其他方法 ====================

void SettingWidget::selectMenuBackground() {
    QString defaultDir = QString::fromStdString(ResourceUtils::getPath("images/"));
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择背景图片", defaultDir, "图片文件 (*.png *.jpg *.jpeg *.bmp)"
    );
    if (!filePath.isEmpty()) {
        currentBgPath = filePath;
        QPixmap bgPixmap(filePath);
        if (!bgPixmap.isNull()) {
            bgPreviewLabel->setPixmap(bgPixmap.scaled(bgPreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

void SettingWidget::initParticles() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> leafSizeDist(0.015f, 0.04f);
    std::uniform_real_distribution<float> cloudSizeDist(0.02f, 0.06f);
    std::uniform_real_distribution<float> speedDist(0.005f, 0.008f);
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> angleSpeedDist(-0.8f, 0.8f);
    std::uniform_real_distribution<float> opacityDist(0.4f, 0.7f);
    std::uniform_real_distribution<float> driftDist(-0.0003f, 0.0003f);

    for (int i = 0; i < 10; ++i) {
        Particle p;
        p.pos = QPointF(posDist(gen), posDist(gen));
        p.size = QSizeF(leafSizeDist(gen) * width(), leafSizeDist(gen) * height() * 1.5f);
        p.angle = angleDist(gen);
        p.speed = speedDist(gen) * 1.2f;
        p.angleSpeed = angleSpeedDist(gen);
        p.opacity = opacityDist(gen);
        p.driftX = driftDist(gen);
        p.driftY = driftDist(gen);
        p.type = 0;
        particles.append(p);
    }

    for (int i = 0; i < 5; ++i) {
        Particle p;
        p.pos = QPointF(posDist(gen), posDist(gen) * 0.6f);
        p.size = QSizeF(cloudSizeDist(gen) * 2 * width(), cloudSizeDist(gen) * 1.2f * height());
        p.angle = 0;
        p.speed = speedDist(gen) * 0.8f;
        p.angleSpeed = 0;
        p.opacity = opacityDist(gen) * 0.6f;
        p.driftX = driftDist(gen) * 0.6f;
        p.driftY = driftDist(gen) * 0.3f;
        p.type = 1;
        particles.append(p);
    }
}

void SettingWidget::updateAnimation() {
    animTime += 0.016f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> opacityDist(0.4f, 0.7f);

    for (auto& p : particles) {
        p.pos.rx() -= p.speed;
        p.pos.ry() += p.driftY;
        p.pos.rx() += p.driftX;

        if (p.type == 0) {
            p.angle += p.angleSpeed;
            if (p.angle < 0) p.angle += 360;
            if (p.angle > 360) p.angle -= 360;
        }

        if (p.pos.x() < -0.1f) {
            p.pos.rx() = 1.1f;
            p.pos.ry() = posDist(gen);
            p.opacity = opacityDist(gen);
        }

        if (p.pos.y() < 0) p.pos.ry() = 0;
        if (p.pos.y() > 1.0f) p.pos.ry() = 1.0f;
    }

    update();
}

void SettingWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);

    QColor bgColor(255, 160, 60, 80);
    p.fillRect(rect(), bgColor);

    QString bgPath = QString::fromStdString(ResourceUtils::getPath("images/setting_bg.png"));
    QPixmap bgPixmap(bgPath);
    if (!bgPixmap.isNull()) {
        p.drawPixmap(rect(), bgPixmap.scaled(rect().size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }

    p.setRenderHint(QPainter::Antialiasing);
    for (const auto& paticle : particles) {
        QPointF drawPos(paticle.pos.x() * width(), paticle.pos.y() * height());
        p.save();

        p.translate(drawPos);
        if (paticle.type == 0) {
            p.rotate(paticle.angle);
        }

        p.setOpacity(paticle.opacity);

        if (paticle.type == 0) {
            QPainterPath leafPath;
            QRectF leafRect(-paticle.size.width()/2, -paticle.size.height()/2, paticle.size.width(), paticle.size.height());
            leafPath.addEllipse(leafRect);

            QLinearGradient leafGrad(leafRect.topLeft(), leafRect.bottomRight());
            leafGrad.setColorAt(0.0, QColor(255, 160, 60, 220));
            leafGrad.setColorAt(1.0, QColor(255, 120, 30, 220));
            p.setBrush(leafGrad);
            p.setPen(Qt::NoPen);
            p.drawPath(leafPath);
        } else {
            QPainterPath cloudPath;
            QRectF cloudRect(-paticle.size.width()/2, -paticle.size.height()/2, paticle.size.width(), paticle.size.height());
            cloudPath.addRoundedRect(cloudRect, cloudRect.height()/2, cloudRect.height()/2);

            QLinearGradient cloudGrad(cloudRect.topLeft(), cloudRect.bottomRight());
            cloudGrad.setColorAt(0.0, QColor(255, 245, 230, 120));
            cloudGrad.setColorAt(1.0, QColor(255, 230, 200, 100));
            p.setBrush(cloudGrad);
            p.setPen(Qt::NoPen);
            p.drawPath(cloudPath);
        }

        p.restore();
    }

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
