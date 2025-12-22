#include "SettingWidget.h"
#include "../GameWindow.h"
#include "../../utils/BackgroundManager.h"
#include "../../utils/ResourceUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStyleOption>
#include <QPainter>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <random>
#include <cmath>
#include <QGraphicsDropShadowEffect>
#include "MenuWidget.h"
#include "../../utils/BGMManager.h"
// 修复点1：添加QSettings头文件包含（必须）
#include <QSettings>

// 静态成员初始化（不变）
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
    QString defaultBg = QString::fromStdString(
        ResourceUtils::getPath(BackgroundManager::instance().getFinalWidgetBackground())
    );
    return settings.value("Image/MenuBg", defaultBg).toString();
}

SettingWidget::SettingWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow), animTime(0) {
    settings = new QSettings("GemMatch", "Settings");
    setWindowTitle("设置");

    // ========== 仅修改点1：恢复窗口大小调整功能（移除强制固定，保留原有尺寸逻辑） ==========
    setMinimumSize(800, 600);
    resize(1600, 1000);

    // ====================== 1. 初始化动画相关（不变） ======================
    initParticles();
    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &SettingWidget::updateAnimation);
    animTimer->start(33);

    // ====================== 2. 初始化所有控件（不变） ======================
    // 音乐设置控件
    bgMusicSlider = new QSlider(Qt::Horizontal, this);
    eliminateSoundSlider = new QSlider(Qt::Horizontal, this);
    bgMusicEnableBox = new QCheckBox("启用", this);
    eliminateSoundEnableBox = new QCheckBox("启用", this);
    QLabel* bgMusicLabel = new QLabel("背景音乐", this);
    QLabel* eliminateSoundLabel = new QLabel("消除音效", this);
    bgVolLabel = new QLabel("50%", this);
    eliminateVolLabel = new QLabel("50%", this);
    eliminateSoundSelectLabel = new QLabel("消除音效", this);
    eliminateSoundCombo = new QComboBox(this);

    // 图像设置控件
    resolutionCombo = new QComboBox(this);
    qualityCombo = new QComboBox(this);
    selectBgBtn = new QPushButton("选择图片", this);
    bgPreviewLabel = new QLabel(this);
    QLabel* resolutionLabel = new QLabel("分辨率", this);
    QLabel* qualityLabel = new QLabel("画质", this);
    QLabel* bgLabel = new QLabel("菜单背景图", this);

    // 游戏设置控件
    switchInterfaceBtn = new QPushButton("切换备用界面（预留）", this);
    QLabel* gameTipLabel = new QLabel("游戏功能设置", this);

    // 宝石风格接口控件
    QLabel* gemStyleLabel = new QLabel("宝石风格", this);
    gemStyleCombo = new QComboBox(this);

    // 保存按钮
    saveBtn = new QPushButton("保存设置", this);
    // 返回按钮
    backBtn = new QPushButton("返回菜单", this);

    // 标签页容器
    tabWidget = new QTabWidget(this);

    // ====================== 3. 美化控件样式（修复点2：修改复选框样式表，移除无效的dot伪元素） ======================
    // 标签样式（保留）
    QList<QLabel*> labels = {bgMusicLabel, eliminateSoundLabel, bgVolLabel, eliminateVolLabel,
                             resolutionLabel, qualityLabel, bgLabel, gameTipLabel, gemStyleLabel, eliminateSoundSelectLabel};
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

    // 修复点2：修改复选框样式表，移除无效的dot伪元素，改用Qt标准的勾选样式绘制
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
            /* 修复：添加勾选的对勾绘制（使用Qt内置的对勾图标或手动绘制） */
            image: url(:/qt-project.org/styles/commonstyle/images/checkbox-checked.png);
            /* 如果没有内置图标，也可以用下面的方式绘制对勾（二选一）：
            image: svg('<svg width="24" height="24" viewBox="0 0 24 24"><path d="M5 13l4 4L19 7" stroke="white" stroke-width="2" fill="none"/></svg>');
            */
        }
        /* 移除无效的dot伪元素 */
    )";
    bgMusicEnableBox->setStyleSheet(checkBoxStyle);
    eliminateSoundEnableBox->setStyleSheet(checkBoxStyle);
    bgMusicEnableBox->setFixedHeight(28);
    eliminateSoundEnableBox->setFixedHeight(28);

    // 滑块样式（新增：滑块禁用样式，可选）
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
            box-shadow: 0 0 10px rgba(255, 160, 60, 0.8);
        }
        QSlider::handle:horizontal:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FFC14F, stop:1 #FFA500);
            transform: scale(1.1);
        }
        /* 新增：滑块禁用时的样式 */
        QSlider:disabled::groove:horizontal {
            background: rgba(255, 160, 60, 0.1);
        }
        QSlider:disabled::handle:horizontal {
            background: rgba(255, 160, 60, 0.5);
            border: 2px solid rgba(255, 245, 230, 0.5);
            box-shadow: none;
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

    // 下拉框样式（保留）
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
            min-width: 240px;
            height: 40px;
        }
        QComboBox::drop-down {
            border-left: 2px solid #FFF5E6;
            border-top-right-radius: 4px;
            border-bottom-right-radius: 4px;
            width: 40px;
        }
        QComboBox::down-arrow {
            image: url(:/images/arrow_down_orange.png);
            width: 16px;
            height: 16px;
        }
        QComboBox::item {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 rgba(255, 120, 30, 0.8), 
                                       stop:1 rgba(255, 100, 20, 0.9));
            color: #FFF5E6;
            padding: 6px 12px;
            font-size: 14px;
            height: 40px;
        }
        QComboBox::item:selected {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FF8C00, stop:1 #FF7F24);
            color: #FFF5E6;
        }
        QComboBox::item:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 rgba(255, 140, 40, 0.6), 
                                       stop:1 rgba(255, 120, 30, 0.6));
        }
    )";
    resolutionCombo->setStyleSheet(comboStyle);
    qualityCombo->setStyleSheet(comboStyle);
    gemStyleCombo->setStyleSheet(comboStyle);
    resolutionCombo->addItems({"1280x720", "1600x1000", "1920x1080", "2560x1440"});
    qualityCombo->addItems({"低", "中", "高", "极致"});
    gemStyleCombo->addItems({"默认风格", "风格一", "风格二", "自定义"});
    eliminateSoundCombo->setStyleSheet(comboStyle);
    eliminateSoundCombo->addItems({"Manbo", "原版"});

    // 按钮样式（保留）
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
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2), 
                        0 0 15px rgba(255, 160, 60, 0.3);
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FFA54F, stop:1 #FF8C00);
            box-shadow: 0 6px 12px rgba(0, 0, 0, 0.3), 
                        0 0 20px rgba(255, 180, 80, 0.5);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FF7F24, stop:1 #FF6347);
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
        }
    )";
    selectBgBtn->setStyleSheet(btnStyle);
    switchInterfaceBtn->setStyleSheet(btnStyle);

    // 保存按钮样式（保留）
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
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2), 
                        0 0 15px rgba(255, 160, 60, 0.4);
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FFC14F, stop:1 #FFA54F);
            box-shadow: 0 6px 12px rgba(0, 0, 0, 0.3), 
                        0 0 20px rgba(255, 180, 80, 0.6);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #FF8C00, stop:1 #FF7F24);
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
        }
    )");
    saveBtn->setFixedSize(200, 44);

    // 返回按钮样式（保留）
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
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2), 
                        0 0 15px rgba(100, 200, 255, 0.3);
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, 
                                       stop:0 #B0E0E6, stop:1 #5F9EA0);
            box-shadow: 0 6px 12px rgba(0, 0, 0, 0.3), 
                        0 0 20px rgba(150, 220, 255, 0.5);
        }
    )");
    backBtn->setFixedSize(200, 44);

    // 背景预览标签（保留）
    bgPreviewLabel->setFixedSize(160, 96);
    bgPreviewLabel->setStyleSheet(R"(
        border: 2px solid #FFA54F;
        border-radius: 4px;
        box-shadow: 0 0 10px rgba(255, 160, 60, 0.5);
        background: rgba(255, 245, 230, 0.1);
    )");

    // 标签页样式（保留）
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
            color: #FFF5E6;
            font-weight: 500;
            box-shadow: 0 -2px 5px rgba(255, 160, 60, 0.3);
        }
        QTabBar::tab:hover:!selected {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1, 
                                       stop:0 rgba(255, 180, 80, 0.4), 
                                       stop:1 rgba(255, 160, 60, 0.6));
        }
    )");
    tabWidget->setMinimumHeight(500);

    // ====================== 4. 构建布局（保留） ======================
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(60, 20, 60, 20);
    mainLayout->setSpacing(20);

    // 音乐设置标签页（保留）
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
    musicLayout->addLayout(bgMusicLayout);
    musicLayout->addLayout(eliminateSoundLayout);
    musicLayout->addStretch();
    QHBoxLayout* eliminateSoundSelectLayout = new QHBoxLayout();
    eliminateSoundSelectLayout->addWidget(eliminateSoundSelectLabel);
    eliminateSoundSelectLayout->addSpacing(20);
    eliminateSoundSelectLayout->addWidget(eliminateSoundCombo);
    eliminateSoundSelectLayout->addStretch();
    musicLayout->insertLayout(2, eliminateSoundSelectLayout);

    // 图像设置标签页（保留）
    QWidget* imageWidget = new QWidget();
    QVBoxLayout* imageLayout = new QVBoxLayout(imageWidget);
    imageLayout->setSpacing(18);
    QHBoxLayout* resolutionLayout = new QHBoxLayout();
    resolutionLayout->addWidget(resolutionLabel);
    resolutionLayout->addSpacing(20);
    resolutionLayout->addWidget(resolutionCombo);
    resolutionLayout->addStretch();
    QHBoxLayout* qualityLayout = new QHBoxLayout();
    qualityLayout->addWidget(qualityLabel);
    qualityLayout->addSpacing(20);
    qualityLayout->addWidget(qualityCombo);
    qualityLayout->addStretch();
    QVBoxLayout* bgLayout = new QVBoxLayout();
    QHBoxLayout* bgCtrlLayout = new QHBoxLayout();
    bgCtrlLayout->addWidget(selectBgBtn);
    bgCtrlLayout->addSpacing(20);
    bgCtrlLayout->addWidget(bgPreviewLabel);
    bgCtrlLayout->addStretch();
    bgLayout->addWidget(bgLabel);
    bgLayout->addLayout(bgCtrlLayout);
    imageLayout->addLayout(resolutionLayout);
    imageLayout->addLayout(qualityLayout);
    imageLayout->addLayout(bgLayout);
    imageLayout->addStretch();

    // 游戏设置标签页（保留）
    QWidget* gameWidget = new QWidget();
    QVBoxLayout* gameLayout = new QVBoxLayout(gameWidget);
    gameLayout->setSpacing(18);
    gameLayout->addWidget(gameTipLabel);
    gameLayout->addWidget(switchInterfaceBtn);
    QHBoxLayout* gemStyleLayout = new QHBoxLayout();
    gemStyleLayout->addWidget(gemStyleLabel);
    gemStyleLayout->addSpacing(20);
    gemStyleLayout->addWidget(gemStyleCombo);
    gemStyleLayout->addStretch();
    gameLayout->addLayout(gemStyleLayout);
    gameLayout->addStretch();

    // 添加标签页（不变）
    tabWidget->addTab(musicWidget, "音乐");
    tabWidget->addTab(imageWidget, "图像");
    tabWidget->addTab(gameWidget, "游戏");

    // 按钮水平布局（保留）
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(30);
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(backBtn);
    btnLayout->addStretch();

    // 按钮容器（保留）
    QWidget* btnWidget = new QWidget(this);
    btnWidget->setLayout(btnLayout);
    btnWidget->setMinimumHeight(80);

    // 主布局（保留）
    mainLayout->addWidget(tabWidget, 1);
    mainLayout->addWidget(btnWidget);

    // ====================== 5. 信号与槽连接（修复点3：添加复选框与滑块的联动，确保交互正常） ======================
    // 音量数值联动
    connect(bgMusicSlider, &QSlider::valueChanged, [=](int val) {
        bgVolLabel->setText(QString("%1%").arg(val));
    });
    connect(eliminateSoundSlider, &QSlider::valueChanged, [=](int val) {
        eliminateVolLabel->setText(QString("%1%").arg(val));
    });

    // 修复点3：添加复选框与滑块的联动（勾选时启用滑块，取消时禁用，同时确保复选框点击响应）
    connect(bgMusicEnableBox, &QCheckBox::toggled, [=](bool checked) {
        bgMusicSlider->setEnabled(checked);
        bgVolLabel->setEnabled(checked);
        // 手动触发一次状态更新（可选，确保视觉同步）
        bgMusicEnableBox->update();
    });
    connect(eliminateSoundEnableBox, &QCheckBox::toggled, [=](bool checked) {
        eliminateSoundSlider->setEnabled(checked);
        eliminateVolLabel->setEnabled(checked);
        eliminateSoundCombo->setEnabled(checked);
        // 手动触发一次状态更新（可选）
        eliminateSoundEnableBox->update();
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

    // 选择背景图按钮
    connect(selectBgBtn, &QPushButton::clicked, this, &SettingWidget::selectMenuBackground);

    // ====================== 6. 加载设置（修复点4：调整顺序，先设置复选框状态，再设置滑块值） ======================
    loadSettings();
}

// 以下函数（不变，仅修改loadSettings函数）
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

    for (int i = 0; i < 20; ++i) {
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

    for (int i = 0; i < 10; ++i) {
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
    animTime += 0.033f;

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

// 修复点4：修改loadSettings函数，调整设置顺序，先设置复选框状态，再设置滑块值
void SettingWidget::loadSettings() {
    if (!bgMusicSlider || !eliminateSoundSlider || !bgMusicEnableBox || !eliminateSoundEnableBox) {
        QMessageBox::critical(this, "错误", "控件未初始化，无法加载设置！");
        return;
    }

    int bgVolume = settings->value("Music/BgVolume", 50).toInt();
    int eliminateVol = settings->value("Music/EliminateVolume", 50).toInt();
    bool bgEnable = settings->value("Music/BgEnable", true).toBool();
    bool eliminateEnable = settings->value("Music/EliminateEnable", true).toBool();
    QString soundType = settings->value("Music/EliminateType", "Manbo").toString();

    // 修复：先设置复选框状态，再设置滑块值
    bgMusicEnableBox->setChecked(bgEnable);
    eliminateSoundEnableBox->setChecked(eliminateEnable);
    // 再设置滑块值，并同步启用状态
    bgMusicSlider->setValue(bgVolume);
    bgMusicSlider->setEnabled(bgEnable);
    eliminateSoundSlider->setValue(eliminateVol);
    eliminateSoundSlider->setEnabled(eliminateEnable);
    // 设置下拉框
    eliminateSoundCombo->setCurrentText(soundType);
    eliminateSoundCombo->setEnabled(eliminateEnable);
    // 更新标签
    bgVolLabel->setText(QString("%1%").arg(bgVolume));
    bgVolLabel->setEnabled(bgEnable);
    eliminateVolLabel->setText(QString("%1%").arg(eliminateVol));
    eliminateVolLabel->setEnabled(eliminateEnable);

    // 图像设置加载（不变）
    QString resolution = settings->value("Image/Resolution", "1600x1000").toString();
    QString quality = settings->value("Image/Quality", "中").toString();
    QString defaultBg = QString::fromStdString(
        ResourceUtils::getPath(BackgroundManager::instance().getFinalWidgetBackground())
    );
    currentBgPath = settings->value("Image/MenuBg", defaultBg).toString();
    resolutionCombo->setCurrentText(resolution);
    qualityCombo->setCurrentText(quality);

    QPixmap bgPixmap(currentBgPath);
    if (!bgPixmap.isNull()) {
        bgPreviewLabel->setPixmap(bgPixmap.scaled(bgPreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void SettingWidget::saveSettings() {
    if (!bgMusicSlider || !eliminateSoundSlider || !bgMusicEnableBox || !eliminateSoundEnableBox) {
        QMessageBox::critical(this, "错误", "控件未初始化，无法保存设置！");
        return;
    }

    settings->setValue("Music/BgVolume", bgMusicSlider->value());
    settings->setValue("Music/EliminateVolume", eliminateSoundSlider->value());
    settings->setValue("Music/BgEnable", bgMusicEnableBox->isChecked());
    settings->setValue("Music/EliminateEnable", eliminateSoundEnableBox->isChecked());
    settings->setValue("Music/EliminateType", eliminateSoundCombo->currentText());

    settings->setValue("Image/Resolution", resolutionCombo->currentText());
    settings->setValue("Image/Quality", qualityCombo->currentText());
    settings->setValue("Image/MenuBg", currentBgPath);

    // 应用背景音乐设置
    BGMManager::instance().setVolume(bgMusicSlider->value());
    if (bgMusicEnableBox->isChecked()) {
        BGMManager::instance().resume();
    } else {
        BGMManager::instance().pause();
    }

    emit backgroundImageChanged(currentBgPath);
    QMessageBox::information(this, "成功", "设置已保存！");
}

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


