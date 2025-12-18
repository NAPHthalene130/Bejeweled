#include "SettingWidget.h"
#include "../GameWindow.h"
#include "../../utils/ResourceUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStyleOption>
#include <QPainter>
#include <QDir>
#include <QMessageBox>
#include "MenuWidget.h"

// 静态成员初始化
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
    // 修复：std::string转QString
    QString defaultBg = QString::fromStdString(ResourceUtils::getPath("images/default_bg.png"));
    return settings.value("Image/MenuBg", defaultBg).toString();
}

SettingWidget::SettingWidget(QWidget* parent, GameWindow* gameWindow)
    : QWidget(parent), gameWindow(gameWindow) {
    settings = new QSettings("GemMatch", "Settings");
    setWindowTitle("设置");
    setMinimumSize(600, 500);
    resize(600, 500); 

    // ====================== 1. 初始化所有控件（关键：解决空指针问题） ======================
    // 音乐设置控件
    bgMusicSlider = new QSlider(Qt::Horizontal, this);
    eliminateSoundSlider = new QSlider(Qt::Horizontal, this);
    bgMusicEnableBox = new QCheckBox("启用", this);
    eliminateSoundEnableBox = new QCheckBox("启用", this);
    QLabel* bgMusicLabel = new QLabel("背景音乐", this);
    QLabel* eliminateSoundLabel = new QLabel("消除音效", this);
    QLabel* bgVolLabel = new QLabel("50%", this);
    QLabel* eliminateVolLabel = new QLabel("50%", this);

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
    // 新增：宝石风格接口控件
    QLabel* gemStyleLabel = new QLabel("宝石风格", this);
    QComboBox* gemStyleCombo = new QComboBox(this);
    
    // 保存按钮
    QPushButton* saveBtn = new QPushButton("保存设置", this);
    // 返回按钮
    QPushButton* backBtn = new QPushButton("返回菜单", this);
    backBtn->setStyleSheet(R"(
        QPushButton { background: #3498db; color: white; border: none; padding: 12px 0; font-size: 16px; border-radius: 4px; }
        QPushButton:hover { background: #5dade2; }
    )");
    // 标签页容器
    QTabWidget* tabWidget = new QTabWidget(this);

    // ====================== 2. 设置控件样式和属性 ======================
    // 滑块样式（紫色主题）
    QString sliderStyle = R"(
        QSlider::groove:horizontal { background: rgba(255,255,255,0.2); height: 8px; border-radius: 4px; }
        QSlider::handle:horizontal { 
            background: #8040C0; 
            border: 2px solid white; 
            width: 20px; 
            height: 20px; 
            margin: -6px 0; 
            border-radius: 10px; 
        }
    )";
    bgMusicSlider->setStyleSheet(sliderStyle);
    eliminateSoundSlider->setStyleSheet(sliderStyle);
    bgMusicSlider->setRange(0, 100);
    eliminateSoundSlider->setRange(0, 100);

    // 文字颜色（白色）
    QList<QLabel*> labels = {bgMusicLabel, eliminateSoundLabel, bgVolLabel, eliminateVolLabel,
                             resolutionLabel, qualityLabel, bgLabel, gameTipLabel, gemStyleLabel};
    for (QLabel* label : labels) {
        label->setStyleSheet("color: white; font-size: 14px;");
    }
    bgVolLabel->setFixedWidth(40);
    bgVolLabel->setAlignment(Qt::AlignCenter);
    eliminateVolLabel->setFixedWidth(40);
    eliminateVolLabel->setAlignment(Qt::AlignCenter);

    // 复选框样式
    bgMusicEnableBox->setStyleSheet("color: white;");
    eliminateSoundEnableBox->setStyleSheet("color: white;");

    // 下拉框样式
    QString comboStyle = R"(
        QComboBox { background: rgba(255,255,255,0.1); color: white; border: 1px solid #8040C0; padding: 5px; }
        QComboBox::drop-down { border-left: 1px solid #8040C0; }
        QComboBox::item { background: rgba(0,0,0,0.7); color: white; }
    )";
    resolutionCombo->setStyleSheet(comboStyle);
    qualityCombo->setStyleSheet(comboStyle);
    resolutionCombo->addItems({"1280x720", "1600x1000", "1920x1080", "2560x1440"});
    qualityCombo->addItems({"低", "中", "高", "极致"});
    // 添加预留的风格选项
    gemStyleCombo->addItems({"默认风格", "风格一", "风格二", "自定义"});
    // 设置样式（与其他下拉框保持一致）
    gemStyleCombo->setStyleSheet(comboStyle);
    // 绑定成员变量
    this->gemStyleLabel = gemStyleLabel;
    this->gemStyleCombo = gemStyleCombo;


    // 按钮样式（紫色）
    QString btnStyle = R"(
        QPushButton { background: #8040C0; color: white; border: none; padding: 8px 15px; border-radius: 4px; }
        QPushButton:hover { background: #9966CC; }
    )";
    selectBgBtn->setStyleSheet(btnStyle);
    switchInterfaceBtn->setStyleSheet(btnStyle);
    saveBtn->setStyleSheet(R"(
        QPushButton { background: #8040C0; color: white; border: none; padding: 12px 0; font-size: 16px; border-radius: 4px; }
        QPushButton:hover { background: #9966CC; }
    )");

    // 背景预览标签
    bgPreviewLabel->setFixedSize(100, 60);
    bgPreviewLabel->setStyleSheet("border: 1px solid #8040C0;");

    // 标签页样式
    tabWidget->setStyleSheet(R"(
        QTabWidget::pane { border: 1px solid #8040C0; }
        QTabBar::tab { 
            background: rgba(128, 64, 192, 0.2); 
            color: white; 
            padding: 10px 20px; 
            margin-right: 5px;
        }
        QTabBar::tab:selected { 
            background: #8040C0; 
            color: white;
        }
    )");

    // ====================== 3. 构建布局 ======================
    // 整体布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // 1. 音乐设置标签页
    QWidget* musicWidget = new QWidget();
    QVBoxLayout* musicLayout = new QVBoxLayout(musicWidget);
    musicLayout->setSpacing(15);

    // 背景音乐布局
    QHBoxLayout* bgMusicLayout = new QHBoxLayout();
    bgMusicLayout->addWidget(bgMusicLabel);
    bgMusicLayout->addWidget(bgMusicEnableBox);
    bgMusicLayout->addWidget(bgMusicSlider);
    bgMusicLayout->addWidget(bgVolLabel);

    // 消除音效布局
    QHBoxLayout* eliminateSoundLayout = new QHBoxLayout();
    eliminateSoundLayout->addWidget(eliminateSoundLabel);
    eliminateSoundLayout->addWidget(eliminateSoundEnableBox);
    eliminateSoundLayout->addWidget(eliminateSoundSlider);
    eliminateSoundLayout->addWidget(eliminateVolLabel);

    musicLayout->addLayout(bgMusicLayout);
    musicLayout->addLayout(eliminateSoundLayout);
    musicLayout->addStretch();

    // 2. 图像设置标签页
    QWidget* imageWidget = new QWidget();
    QVBoxLayout* imageLayout = new QVBoxLayout(imageWidget);
    imageLayout->setSpacing(15);

    // 分辨率布局
    QHBoxLayout* resolutionLayout = new QHBoxLayout();
    resolutionLayout->addWidget(resolutionLabel);
    resolutionLayout->addWidget(resolutionCombo);
    resolutionLayout->addStretch();

    // 画质布局
    QHBoxLayout* qualityLayout = new QHBoxLayout();
    qualityLayout->addWidget(qualityLabel);
    qualityLayout->addWidget(qualityCombo);
    qualityLayout->addStretch();

    // 背景图布局
    QVBoxLayout* bgLayout = new QVBoxLayout();
    QHBoxLayout* bgCtrlLayout = new QHBoxLayout();
    bgCtrlLayout->addWidget(selectBgBtn);
    bgCtrlLayout->addSpacing(10);
    bgCtrlLayout->addWidget(bgPreviewLabel);
    bgCtrlLayout->addStretch();
    bgLayout->addWidget(bgLabel);
    bgLayout->addLayout(bgCtrlLayout);

    imageLayout->addLayout(resolutionLayout);
    imageLayout->addLayout(qualityLayout);
    imageLayout->addLayout(bgLayout);
    imageLayout->addStretch();

    // 3. 游戏设置标签页（预留）
    QWidget* gameWidget = new QWidget();
    QVBoxLayout* gameLayout = new QVBoxLayout(gameWidget);
    gameLayout->setSpacing(15);
    gameLayout->addWidget(gameTipLabel);
    gameLayout->addWidget(switchInterfaceBtn);

    // 新增：添加宝石风格选择布局
    QHBoxLayout* gemStyleLayout = new QHBoxLayout();
    gemStyleLayout->addWidget(gemStyleLabel);
    gemStyleLayout->addWidget(gemStyleCombo);
    gemStyleLayout->addStretch(); // 右侧留白
    gameLayout->addLayout(gemStyleLayout);

    gameLayout->addStretch();

    // 添加标签页
    tabWidget->addTab(musicWidget, "音乐");
    tabWidget->addTab(imageWidget, "图像");
    tabWidget->addTab(gameWidget, "游戏");

    // 添加到主布局
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(saveBtn);
    mainLayout->addWidget(backBtn);

    

    // ====================== 4. 信号与槽连接 ======================
    // 音量数值联动
    connect(bgMusicSlider, &QSlider::valueChanged, [=](int val) {
        bgVolLabel->setText(QString("%1%").arg(val));
    });
    connect(eliminateSoundSlider, &QSlider::valueChanged, [=](int val) {
        eliminateVolLabel->setText(QString("%1%").arg(val));
    });

    // 保存按钮
    connect(saveBtn, &QPushButton::clicked, this, &SettingWidget::saveSettings);

    connect(backBtn, &QPushButton::clicked, this, [this]() {
        this->hide();
        if (this->gameWindow) {
            // 直接使用隐式转换（MenuWidget* -> QWidget*）
            this->gameWindow->switchWidget(this->gameWindow->getMenuWidget());
        }
    });

    // 选择背景图按钮
    connect(selectBgBtn, &QPushButton::clicked, this, &SettingWidget::selectMenuBackground);

    // ====================== 5. 最后加载设置（关键：初始化控件后再调用） ======================
    loadSettings();
}

// 加载设置
void SettingWidget::loadSettings() {
    // 空指针检查（健壮性补充）
    if (!bgMusicSlider || !eliminateSoundSlider || !bgMusicEnableBox || !eliminateSoundEnableBox) {
        QMessageBox::critical(this, "错误", "控件未初始化，无法加载设置！");
        return;
    }

    // 音乐设置
    int bgVol = settings->value("Music/BgVolume", 50).toInt();
    int eliminateVol = settings->value("Music/EliminateVolume", 50).toInt();
    bool bgEnable = settings->value("Music/BgEnable", true).toBool();
    bool eliminateEnable = settings->value("Music/EliminateEnable", true).toBool();
    bgMusicSlider->setValue(bgVol);
    eliminateSoundSlider->setValue(eliminateVol);
    bgMusicEnableBox->setChecked(bgEnable);
    eliminateSoundEnableBox->setChecked(eliminateEnable);

    // 图像设置
    QString resolution = settings->value("Image/Resolution", "1600x1000").toString();
    QString quality = settings->value("Image/Quality", "中").toString();
    // 修复：std::string转QString
    QString defaultBg = QString::fromStdString(ResourceUtils::getPath("images/default_bg.png"));
    currentBgPath = settings->value("Image/MenuBg", defaultBg).toString();
    resolutionCombo->setCurrentText(resolution);
    qualityCombo->setCurrentText(quality);

    // 预览背景图
    QPixmap bgPixmap(currentBgPath);
    if (!bgPixmap.isNull()) {
        bgPreviewLabel->setPixmap(bgPixmap.scaled(bgPreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

// 保存设置
void SettingWidget::saveSettings() {
    // 空指针检查
    if (!bgMusicSlider || !eliminateSoundSlider || !bgMusicEnableBox || !eliminateSoundEnableBox) {
        QMessageBox::critical(this, "错误", "控件未初始化，无法保存设置！");
        return;
    }

    // 音乐设置
    settings->setValue("Music/BgVolume", bgMusicSlider->value());
    settings->setValue("Music/EliminateVolume", eliminateSoundSlider->value());
    settings->setValue("Music/BgEnable", bgMusicEnableBox->isChecked());
    settings->setValue("Music/EliminateEnable", eliminateSoundEnableBox->isChecked());

    // 图像设置
    settings->setValue("Image/Resolution", resolutionCombo->currentText());
    settings->setValue("Image/Quality", qualityCombo->currentText());
    settings->setValue("Image/MenuBg", currentBgPath);

    // 发送背景图变更信号
    emit backgroundImageChanged(currentBgPath);

    QMessageBox::information(this, "成功", "设置已保存！");
}

// 选择菜单背景图
void SettingWidget::selectMenuBackground() {
    // 修复：获取资源目录的图片路径
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

// 绘制半透明背景（突出背景图片）
void SettingWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    // 半透明浅紫色背景（突出背景图）
    QColor bgColor(20, 20, 40, 180);
    p.fillRect(rect(), bgColor);
    // 绘制背景图片（第二张图片，路径为resources/images/setting_bg.png）
    QString bgPath = QString::fromStdString(ResourceUtils::getPath("images/setting_bg.png"));
    QPixmap bgPixmap(bgPath);
    if (!bgPixmap.isNull()) {
        p.drawPixmap(rect(), bgPixmap.scaled(rect().size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}