#include "Gemstone.h"
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <QColor>
#include <QVector3D>
#include <QQuaternion>
#include <QRandomGenerator>
#include <Qt3DRender/QPickingSettings>
#include <QString>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QSettings>

// ============================================================================
// GemstoneModelManager 实现
// ============================================================================

GemstoneModelManager& GemstoneModelManager::instance() {
    static GemstoneModelManager instance;
    return instance;
}

GemstoneModelManager::GemstoneModelManager() 
    : QObject(nullptr)
    , m_currentStyle(GemstoneStyle::Builtin)
    , m_filePattern("gem_type_%1.obj") 
{
    // 初始化风格目录映射
    initStyleDirectories();
    
    // 从设置加载当前风格
    QSettings settings("GemMatch", "Settings");
    QString savedStyle = settings.value("Game/GemStyle", "几何体").toString();
    m_currentStyle = nameToStyle(savedStyle);
    
    // 尝试自动检测资源目录
    QStringList searchPaths;
    searchPaths << QCoreApplication::applicationDirPath() + "/resources";
    searchPaths << QCoreApplication::applicationDirPath() + "/assets";
    searchPaths << QDir::currentPath() + "/resources";
    searchPaths << QDir::currentPath() + "/assets";
    
    QDir parentDir(QCoreApplication::applicationDirPath());
    parentDir.cdUp();
    searchPaths << parentDir.path() + "/resources";
    searchPaths << parentDir.path() + "/assets";
    
    parentDir.cdUp();
    searchPaths << parentDir.path() + "/resources";
    searchPaths << parentDir.path() + "/assets";
    
#ifdef PROJECT_SOURCE_DIR
    searchPaths << QString(PROJECT_SOURCE_DIR) + "/resources";
    searchPaths << QString(PROJECT_SOURCE_DIR) + "/assets";
#endif
    
    for (const QString& path : searchPaths) {
        QDir dir(path);
        if (dir.exists()) {
            m_resourcesDirectory = path;
            qDebug() << "[GemstoneModelManager] Found resources directory:" << path;
            break;
        }
    }
    
    if (m_resourcesDirectory.isEmpty()) {
        qDebug() << "[GemstoneModelManager] No resources directory found";
    }
    
    // 扫描所有风格的模型
    scanAllStyles();
}

void GemstoneModelManager::initStyleDirectories() {
    m_styleDirectories[GemstoneStyle::Builtin] = "";  // 内置不需要目录
    m_styleDirectories[GemstoneStyle::Gemstones] = "gemstones";
    m_styleDirectories[GemstoneStyle::Planets] = "planets";
    m_styleDirectories[GemstoneStyle::FastFood] = "fastfood";
    m_styleDirectories[GemstoneStyle::Custom1] = "custom1";
    m_styleDirectories[GemstoneStyle::Custom2] = "custom2";
}

QString GemstoneModelManager::styleToName(GemstoneStyle style) {
    switch (style) {
        case GemstoneStyle::Builtin: return "几何体";
        case GemstoneStyle::Gemstones: return "宝石";
        case GemstoneStyle::Planets: return "八大行星";
        case GemstoneStyle::FastFood: return "美食";
        case GemstoneStyle::Custom1: return "自定义风格1";
        case GemstoneStyle::Custom2: return "自定义风格2";
        default: return "几何体";
    }
}

GemstoneStyle GemstoneModelManager::nameToStyle(const QString& name) {
    if (name == "几何体" || name == "Builtin") return GemstoneStyle::Builtin;
    if (name == "宝石" || name == "Gemstones") return GemstoneStyle::Gemstones;
    if (name == "八大行星" || name == "Planets") return GemstoneStyle::Planets;
    if (name == "美食" || name == "FastFood") return GemstoneStyle::FastFood;
    if (name == "自定义风格1" || name == "Custom1") return GemstoneStyle::Custom1;
    if (name == "自定义风格2" || name == "Custom2") return GemstoneStyle::Custom2;
    return GemstoneStyle::Builtin;
}

void GemstoneModelManager::setCurrentStyle(GemstoneStyle style) {
    if (m_currentStyle != style) {
        m_currentStyle = style;
        
        // 保存到设置
        QSettings settings("GemMatch", "Settings");
        settings.setValue("Game/GemStyle", styleToName(style));
        
        qDebug() << "[GemstoneModelManager] Style changed to:" << styleToName(style);
        
        emit styleChanged(style);
    }
}

void GemstoneModelManager::setCurrentStyleByName(const QString& styleName) {
    setCurrentStyle(nameToStyle(styleName));
}

GemstoneStyle GemstoneModelManager::getCurrentStyle() const {
    return m_currentStyle;
}

QString GemstoneModelManager::getCurrentStyleName() const {
    return styleToName(m_currentStyle);
}

QStringList GemstoneModelManager::getAvailableStyles() const {
    QStringList styles;
    styles << "几何体";  // 内置总是可用
    
    // 检查其他风格目录是否存在模型
    if (m_modelCache.contains(GemstoneStyle::Gemstones) && 
        !m_modelCache[GemstoneStyle::Gemstones].isEmpty()) {
        styles << "宝石";
    }
    
    if (m_modelCache.contains(GemstoneStyle::Planets) && 
        !m_modelCache[GemstoneStyle::Planets].isEmpty()) {
        styles << "八大行星";
    }
    
    if (m_modelCache.contains(GemstoneStyle::FastFood) && 
        !m_modelCache[GemstoneStyle::FastFood].isEmpty()) {
        styles << "美食";
    }
    
    if (m_modelCache.contains(GemstoneStyle::Custom1) && 
        !m_modelCache[GemstoneStyle::Custom1].isEmpty()) {
        styles << "自定义风格1";
    }
    
    if (m_modelCache.contains(GemstoneStyle::Custom2) && 
        !m_modelCache[GemstoneStyle::Custom2].isEmpty()) {
        styles << "自定义风格2";
    }
    
    return styles;
}

void GemstoneModelManager::setResourcesDirectory(const QString& path) {
    if (m_resourcesDirectory != path) {
        m_resourcesDirectory = path;
        scanAllStyles();
        qDebug() << "[GemstoneModelManager] Resources directory set to:" << path;
    }
}

QString GemstoneModelManager::getResourcesDirectory() const {
    return m_resourcesDirectory;
}

QString GemstoneModelManager::getStyleDirectory(GemstoneStyle style) const {
    if (style == GemstoneStyle::Builtin || m_resourcesDirectory.isEmpty()) {
        return QString();
    }
    
    QString subDir = m_styleDirectories.value(style, "");
    if (subDir.isEmpty()) {
        return QString();
    }
    
    return m_resourcesDirectory + "/" + subDir;
}

QString GemstoneModelManager::getModelPath(int type) const {
    return getModelPath(m_currentStyle, type);
}

QString GemstoneModelManager::getModelPath(GemstoneStyle style, int type) const {
    if (style == GemstoneStyle::Builtin) {
        return QString();  // 内置风格不使用外部模型
    }
    
    int normalizedType = type % 8;
    
    if (m_modelCache.contains(style) && 
        m_modelCache[style].contains(normalizedType)) {
        return m_modelCache[style][normalizedType];
    }
    
    // 尝试构造路径
    QString styleDir = getStyleDirectory(style);
    if (!styleDir.isEmpty()) {
        QString filename = m_filePattern.arg(normalizedType);
        QString fullPath = styleDir + "/" + filename;
        if (QFileInfo::exists(fullPath)) {
            return fullPath;
        }
    }
    
    return QString();
}

bool GemstoneModelManager::hasExternalModel(int type) const {
    return hasExternalModel(m_currentStyle, type);
}

bool GemstoneModelManager::hasExternalModel(GemstoneStyle style, int type) const {
    if (style == GemstoneStyle::Builtin) {
        return false;
    }
    return !getModelPath(style, type).isEmpty();
}

int GemstoneModelManager::getModelCount() const {
    if (m_currentStyle == GemstoneStyle::Builtin) {
        return 8;  // 内置8种几何体
    }
    
    if (m_modelCache.contains(m_currentStyle)) {
        return m_modelCache[m_currentStyle].size();
    }
    
    return 0;
}

void GemstoneModelManager::refreshModelCache() {
    scanAllStyles();
    emit modelsReloaded();
}

void GemstoneModelManager::setModelFilePattern(const QString& pattern) {
    m_filePattern = pattern;
    scanAllStyles();
}

QString GemstoneModelManager::getModelFilePattern() const {
    return m_filePattern;
}

void GemstoneModelManager::scanAllStyles() {
    m_modelCache.clear();
    
    // 扫描每个风格目录
    QList<GemstoneStyle> styles = {
        GemstoneStyle::Gemstones,
        GemstoneStyle::Planets,
        GemstoneStyle::FastFood,
        GemstoneStyle::Custom1,
        GemstoneStyle::Custom2
    };
    
    for (GemstoneStyle style : styles) {
        scanStyleDirectory(style);
    }
}

void GemstoneModelManager::scanStyleDirectory(GemstoneStyle style) {
    QString styleDir = getStyleDirectory(style);
    
    if (styleDir.isEmpty()) {
        return;
    }
    
    QDir dir(styleDir);
    if (!dir.exists()) {
        qDebug() << "[GemstoneModelManager] Style directory does not exist:" << styleDir;
        return;
    }
    
    QStringList filters;
    filters << "gem_type_*.obj" << "gem_type_*.gltf" << "gem_type_*.glb";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    QMap<int, QString> typeCache;
    
    for (const QFileInfo& fileInfo : files) {
        QString filename = fileInfo.baseName();
        
        if (filename.startsWith("gem_type_")) {
            QString numStr = filename.mid(9);
            bool ok;
            int type = numStr.toInt(&ok);
            if (ok && type >= 0 && type < 8) {
                typeCache[type] = fileInfo.absoluteFilePath();
                qDebug() << "[GemstoneModelManager] Found" << styleToName(style) 
                         << "model for type" << type << ":" << fileInfo.absoluteFilePath();
            }
        }
    }
    
    if (!typeCache.isEmpty()) {
        m_modelCache[style] = typeCache;
        qDebug() << "[GemstoneModelManager] Scanned" << typeCache.size() 
                 << "models for style:" << styleToName(style);
    }
}

// ============================================================================
// Gemstone 实现
// ============================================================================

Gemstone::Gemstone(int type, std::string style, Qt3DCore::QNode* parent) 
    : Qt3DCore::QEntity(parent), type(type), style(style), m_externalMesh(nullptr) {
    
    m_transform = new Qt3DCore::QTransform(this);
    addComponent(m_transform);

    m_material = new Qt3DExtras::QPhongMaterial(this);
    addComponent(m_material);

    m_mesh = nullptr;

    // 连接全局风格变化信号
    connect(&GemstoneModelManager::instance(), &GemstoneModelManager::styleChanged,
            this, &Gemstone::onGlobalStyleChanged);

    updateAppearance();

    // 设置旋转动画
    m_rotationAnimation = new QPropertyAnimation(m_transform, "rotationY", this);
    m_rotationAnimation->setStartValue(0.0f);
    m_rotationAnimation->setEndValue(360.0f);
    m_rotationAnimation->setDuration(3000 + QRandomGenerator::global()->bounded(2000));
    m_rotationAnimation->setLoopCount(-1);
    m_rotationAnimation->start();

    // 设置对象选择器
    m_picker = new Qt3DRender::QObjectPicker(this);
    m_picker->setHoverEnabled(true);
    m_picker->setDragEnabled(false);
    m_picker->setEnabled(true);
    addComponent(m_picker);

    connect(m_picker, &Qt3DRender::QObjectPicker::pressed, this, [this](Qt3DRender::QPickEvent* event) {
        Q_UNUSED(event);
        emit pickEvent("pressed");
    });

    connect(m_picker, &Qt3DRender::QObjectPicker::clicked, this, [this](Qt3DRender::QPickEvent* event) {
        Q_UNUSED(event);
        emit pickEvent("clicked");
        emit clicked(this);
    });

    connect(m_picker, &Qt3DRender::QObjectPicker::entered, this, [this]() {
        emit pickEvent("entered");
    });

    connect(m_picker, &Qt3DRender::QObjectPicker::exited, this, [this]() {
        emit pickEvent("exited");
    });
}

Gemstone::~Gemstone() {
    // 清理金币图标
    clearCoinIndicator();
    // Qt3D 节点会自动清理
    // Qt3D 节点会自动清理子节点，但未设置父节点的组件需要手动清理
    if (m_mesh) {
        delete m_mesh;
    }
    if (m_externalMesh) {
        delete m_externalMesh;
    }
}

void Gemstone::onGlobalStyleChanged(GemstoneStyle newStyle) {
    Q_UNUSED(newStyle);
    // 风格变化时重新加载模型
    reloadModel();
    clearSpecialEffects();
    clearHintEffects();
}

int Gemstone::getType() const {
    return type;
}

void Gemstone::setType(int type) {
    if (this->type != type) {
        this->type = type;
        updateAppearance();
    }
}

std::string Gemstone::getStyle() const {
    return style;
}

void Gemstone::setStyle(const std::string& style) {
    if (this->style != style) {
        this->style = style;
        updateAppearance();
    }
}

Qt3DCore::QTransform* Gemstone::transform() const {
    return m_transform;
}

bool Gemstone::isUsingExternalModel() const {
    return m_usingExternalModel;
}

void Gemstone::reloadModel() {
    updateAppearance();
}

void Gemstone::updateAppearance() {
    setupMesh();
    setupMaterial();
}

void Gemstone::setupMesh() {
    // 移除旧的网格组件
    if (m_mesh) {
        removeComponent(m_mesh);
        delete m_mesh;
        m_mesh = nullptr;
    }
    
    if (m_externalMesh) {
        removeComponent(m_externalMesh);
        delete m_externalMesh;
        m_externalMesh = nullptr;
    }
    
    m_usingExternalModel = false;

    GemstoneStyle currentStyle = GemstoneModelManager::instance().getCurrentStyle();
    
    // 如果是内置风格，直接使用几何体
    if (currentStyle == GemstoneStyle::Builtin) {
        setupBuiltinMesh();
        return;
    }
    
    // 尝试使用外部模型
    if (GemstoneModelManager::instance().hasExternalModel(type % 8)) {
        setupExternalMesh();
    }
    
    // 如果没有成功加载外部模型，使用内置几何体
    if (!m_usingExternalModel) {
        setupBuiltinMesh();
    }
}

void Gemstone::setupExternalMesh() {
    QString modelPath = GemstoneModelManager::instance().getModelPath(type % 8);
    
    if (modelPath.isEmpty()) {
        return;
    }
    
    m_externalMesh = new Qt3DRender::QMesh();
    QUrl url = QUrl::fromLocalFile(modelPath);
    m_externalMesh->setSource(url);
    
    connect(m_externalMesh, &Qt3DRender::QMesh::statusChanged, this, 
            [this, modelPath](Qt3DRender::QMesh::Status status) {
        switch (status) {
            case Qt3DRender::QMesh::Ready:
                qDebug() << "[Gemstone] External model loaded:" << modelPath;
                m_usingExternalModel = true;
                emit modelLoaded(true);
                break;
            case Qt3DRender::QMesh::Error:
                qDebug() << "[Gemstone] Error loading model:" << modelPath;
                setupBuiltinMesh();
                emit modelLoaded(false);
                break;
            default:
                break;
        }
    });
    
    addComponent(m_externalMesh);
    m_usingExternalModel = true;
}

void Gemstone::setupBuiltinMesh() {
    // 移除旧的网格组件
    if (m_mesh) {
        removeComponent(m_mesh);
        delete m_mesh;
        m_mesh = nullptr;
    }

    // 使用内置几何体
    qDebug() << "[Gemstone] Setting up builtin mesh for type" << type;
    switch (type % 8) {
        case 0: // 球体
        {
            Qt3DExtras::QSphereMesh* mesh = new Qt3DExtras::QSphereMesh(this);
            mesh->setRadius(0.45f);
            mesh->setRings(20); 
            mesh->setSlices(20);
            m_mesh = mesh;
            break;
        }
        case 1: // 立方体
        {
            Qt3DExtras::QCuboidMesh* mesh = new Qt3DExtras::QCuboidMesh(this);
            mesh->setXExtent(0.8f); 
            mesh->setYExtent(0.8f); 
            mesh->setZExtent(0.8f);
            m_mesh = mesh;
            break;
        }
        case 2: // 圆锥体
        {
            Qt3DExtras::QConeMesh* mesh = new Qt3DExtras::QConeMesh(this);
            mesh->setBottomRadius(0.5f);
            mesh->setLength(1.0f);
            mesh->setRings(10); 
            mesh->setSlices(20);
            m_mesh = mesh;
            break;
        }
        case 3: // 圆柱体
        {
            Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh(this);
            mesh->setRadius(0.45f);
            mesh->setLength(0.9f);
            mesh->setRings(10); 
            mesh->setSlices(20);
            m_mesh = mesh;
            break;
        }
        case 4: // 圆环体
        {
            Qt3DExtras::QTorusMesh* mesh = new Qt3DExtras::QTorusMesh(this);
            mesh->setRadius(0.4f);
            mesh->setMinorRadius(0.15f);
            mesh->setRings(20); 
            mesh->setSlices(20);
            m_mesh = mesh;
            break;
        }
        case 5: // 六棱柱
        {
            Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh(this);
            mesh->setRadius(0.5f);
            mesh->setLength(0.8f);
            mesh->setRings(2); 
            mesh->setSlices(6);
            m_mesh = mesh;
            break;
        }
        case 6: // 金字塔
        {
            Qt3DExtras::QConeMesh* mesh = new Qt3DExtras::QConeMesh(this);
            mesh->setBottomRadius(0.5f);
            mesh->setLength(0.9f);
            mesh->setRings(2); 
            mesh->setSlices(4);
            m_mesh = mesh;
            break;
        }
        case 7: // 三棱柱
        {
            Qt3DExtras::QCylinderMesh* mesh = new Qt3DExtras::QCylinderMesh(this);
            mesh->setRadius(0.5f);
            mesh->setLength(0.8f);
            mesh->setRings(2); 
            mesh->setSlices(3);
            m_mesh = mesh;
            break;
        }
        default:
        {
            Qt3DExtras::QSphereMesh* mesh = new Qt3DExtras::QSphereMesh(this);
            mesh->setRadius(0.4f);
            m_mesh = mesh;
            break;
        }
    }
    
    if (m_mesh) {
        addComponent(m_mesh);
    }
}

QColor Gemstone::getGemColor() const {
    GemstoneStyle currentStyle = GemstoneModelManager::instance().getCurrentStyle();
    
    switch (currentStyle) {
        case GemstoneStyle::Builtin:
            return getBuiltinColor();
        case GemstoneStyle::Gemstones:
            return getGemstoneColor();
        case GemstoneStyle::Planets:
            return getPlanetColor();
        case GemstoneStyle::FastFood:
            return getFastFoodColor();
        default:
            return getBuiltinColor();
    }
}

QColor Gemstone::getBuiltinColor() const {
    // 原始几何体颜色
    switch (type % 8) {
        case 0: return QColor(255, 50, 50);   // 红色
        case 1: return QColor(50, 255, 50);   // 绿色
        case 2: return QColor(50, 50, 255);   // 蓝色
        case 3: return QColor(255, 255, 50);  // 黄色
        case 4: return QColor(255, 50, 255);  // 品红色
        case 5: return QColor(50, 255, 255);  // 青色
        case 6: return QColor(255, 150, 50);  // 橙色
        case 7: return QColor(200, 200, 200); // 灰色
        default: return Qt::white;
    }
}

QColor Gemstone::getGemstoneColor() const {
    // 宝石风格颜色
    switch (type % 8) {
        case 0: return QColor(255, 102, 153);   // 粉色水晶
        case 1: return QColor(69, 26, 150);     // 祖母紫 （（（（））））
        case 2: return QColor(26, 153, 204);    // 青蓝水晶
        case 3: return QColor(255, 140, 26);    // 橙色钻石
        case 4: return QColor(51, 217, 89);     // 明亮绿色
        case 5: return QColor(153, 51, 204);    // 紫色水晶
        case 6: return QColor(255, 77, 128);    // 粉色钻石
        case 7: return QColor(255, 217, 51);    // 黄色宝石
        default: return Qt::white;
    }
}

QColor Gemstone::getPlanetColor() const {
    // 八大行星颜色
    switch (type % 8) {
        case 0: return QColor(200, 180, 120);   // 水星 - 黄灰色
        case 1: return QColor(240, 180, 80);    // 金星 - 橙黄色
        case 2: return QColor(65, 145, 220);    // 地球 - 蓝色
        case 3: return QColor(210, 90, 60);     // 火星 - 铁锈红
        case 4: return QColor(220, 150, 90);    // 木星 - 橙褐色
        case 5: return QColor(235, 210, 150);   // 土星 - 淡黄色
        case 6: return QColor(175, 220, 230);   // 天王星 - 浅青蓝
        case 7: return QColor(60, 100, 180);    // 海王星 - 深蓝色
        default: return Qt::white;
    }
}

QColor Gemstone::getFastFoodColor() const {
    // 美食风格颜色
    switch (type % 8) {
        case 0: return QColor(255, 215, 75);    // 薯条 - 金黄色
        case 1: return QColor(215, 140, 65);    // 汉堡 - 面包色
        case 2: return QColor(240, 165, 50);    // 鸡块 - 炸鸡色
        case 3: return QColor(255, 200, 50);    // 蛋挞 - 蛋黄色
        case 4: return QColor(255, 190, 75);    // 披萨 - 芝士黄
        case 5: return QColor(255, 180, 200);   // 冰淇淋 - 粉色
        case 6: return QColor(230, 180, 115);   // 三明治 - 面包色
        case 7: return QColor(215, 155, 90);    // 热狗 - 面包色
        default: return Qt::white;
    }
}

void Gemstone::setupMaterial() {
    QColor color = getGemColor();
    
    m_material->setDiffuse(color);
    m_material->setAmbient(color.darker(150));
    m_material->setSpecular(Qt::white);
    
    // 根据风格调整光泽度
    GemstoneStyle currentStyle = GemstoneModelManager::instance().getCurrentStyle();
    switch (currentStyle) {
        case GemstoneStyle::Gemstones:
            m_material->setShininess(100.0f);  // 宝石更闪亮
            break;
        case GemstoneStyle::Planets:
            m_material->setShininess(30.0f);   // 行星较为哑光
            break;
        case GemstoneStyle::FastFood:
            m_material->setShininess(45.0f);   // 美食适中光泽
            m_material->setAmbient(color.lighter(110));  // 稍微明亮
            break;
        default:
            m_material->setShininess(50.0f);
            break;
    }
}

void Gemstone::setSpecial(bool special) {
    if (this->special != special) {
        this->special = special;
        updateSpecialEffects();
    }
}

void Gemstone::setHint(bool hint) {
    if(this->hint != hint) {
        this -> hint = hint;
        updateHintEffects();
    }
}

bool Gemstone::isSpecial() const {
    return special;
}

bool Gemstone::getCanBeChosen() const {
    return canBeChosen;
}

void Gemstone::setCanBeChosen(bool can) {
    canBeChosen = can;
    if (m_picker) {
        m_picker->setEnabled(can);
    }
}

void Gemstone::updateSpecialEffects() {
    clearSpecialEffects();

    if (!special) {
        return;
    }

    // 光环效果
    m_haloEntity = new Qt3DCore::QEntity(this);
    
    Qt3DExtras::QTorusMesh* haloMesh = new Qt3DExtras::QTorusMesh();
    haloMesh->setRadius(0.7f);
    haloMesh->setMinorRadius(0.05f);
    haloMesh->setRings(30);
    haloMesh->setSlices(30);

    Qt3DExtras::QPhongMaterial* haloMat = new Qt3DExtras::QPhongMaterial();
    haloMat->setDiffuse(QColor(255, 255, 200));
    haloMat->setAmbient(QColor(255, 255, 200));
    haloMat->setSpecular(Qt::white);
    haloMat->setShininess(100.0f);
    
    Qt3DCore::QTransform* haloTransform = new Qt3DCore::QTransform();
    haloTransform->setRotationX(30.0f);

    m_haloEntity->addComponent(haloMesh);
    m_haloEntity->addComponent(haloMat);
    m_haloEntity->addComponent(haloTransform);

    m_haloScaleAnimation = new QPropertyAnimation(haloTransform, "scale3D");
    m_haloScaleAnimation->setStartValue(QVector3D(1.0f, 1.0f, 1.0f));
    m_haloScaleAnimation->setEndValue(QVector3D(1.2f, 1.2f, 1.2f));
    m_haloScaleAnimation->setDuration(1000);
    m_haloScaleAnimation->setLoopCount(-1);
    m_haloScaleAnimation->setEasingCurve(QEasingCurve::InOutSine);
    m_haloScaleAnimation->start();

    // 粒子效果
    m_particlesRoot = new Qt3DCore::QEntity(this);
    int particleCount = 6;
    
    for (int i = 0; i < particleCount; ++i) {
        Qt3DCore::QEntity* pEntity = new Qt3DCore::QEntity(m_particlesRoot);
        
        Qt3DExtras::QSphereMesh* pMesh = new Qt3DExtras::QSphereMesh();
        pMesh->setRadius(0.08f);
        
        Qt3DExtras::QPhongMaterial* pMat = new Qt3DExtras::QPhongMaterial();
        pMat->setDiffuse(QColor(255, 255, 100));
        pMat->setAmbient(QColor(255, 255, 0));
        pMat->setShininess(50.0f);
        
        Qt3DCore::QTransform* pTransform = new Qt3DCore::QTransform();
        
        pEntity->addComponent(pMesh);
        pEntity->addComponent(pMat);
        pEntity->addComponent(pTransform);
        
        m_particleEntities.push_back(pEntity);

        Qt3DCore::QEntity* pivot = new Qt3DCore::QEntity(m_particlesRoot);
        Qt3DCore::QTransform* pivotTransform = new Qt3DCore::QTransform();
        pivot->addComponent(pivotTransform);
        
        pEntity->setParent(pivot);
        pTransform->setTranslation(QVector3D(0.9f, 0.0f, 0.0f));
        
        QPropertyAnimation* orbitAnim = new QPropertyAnimation(pivotTransform, "rotationY");
        orbitAnim->setStartValue(0.0f);
        orbitAnim->setEndValue(360.0f);
        int duration = 1500 + QRandomGenerator::global()->bounded(1500);
        orbitAnim->setDuration(duration);
        orbitAnim->setLoopCount(-1);
        
        float startAngle = (360.0f / particleCount) * i;
        pivotTransform->setRotationY(startAngle);
        orbitAnim->setStartValue(startAngle);
        orbitAnim->setEndValue(startAngle + 360.0f);
        
        float tiltX = QRandomGenerator::global()->bounded(60) - 30;
        float tiltZ = QRandomGenerator::global()->bounded(60) - 30;
        QQuaternion tilt = QQuaternion::fromEulerAngles(tiltX, 0, tiltZ);
        
        Qt3DCore::QEntity* tiltRoot = new Qt3DCore::QEntity(m_particlesRoot);
        Qt3DCore::QTransform* tiltTransform = new Qt3DCore::QTransform();
        tiltTransform->setRotation(tilt);
        tiltRoot->addComponent(tiltTransform);
        
        pivot->setParent(tiltRoot);

        orbitAnim->start();
        m_particleAnimations.push_back(orbitAnim);
    }
}

void Gemstone::updateHintEffects() {
    clearHintEffects();

    if (!hint) {
        return;
    }

    // 粒子效果
    m_particlesRoot_hint = new Qt3DCore::QEntity(this);
    int particleCount = 6;
    
    for (int i = 0; i < particleCount; ++i) {
        Qt3DCore::QEntity* pEntity = new Qt3DCore::QEntity(m_particlesRoot_hint);
        
        Qt3DExtras::QSphereMesh* pMesh = new Qt3DExtras::QSphereMesh();
        pMesh->setRadius(0.08f);
        
        Qt3DExtras::QPhongMaterial* pMat = new Qt3DExtras::QPhongMaterial();
        pMat->setDiffuse(QColor(40, 60, 255));
        pMat->setAmbient(QColor(40, 100, 255));
        pMat->setShininess(50.0f);
        
        Qt3DCore::QTransform* pTransform = new Qt3DCore::QTransform();
        
        pEntity->addComponent(pMesh);
        pEntity->addComponent(pMat);
        pEntity->addComponent(pTransform);
        
        m_particleEntities_hint.push_back(pEntity);

        Qt3DCore::QEntity* pivot = new Qt3DCore::QEntity(m_particlesRoot_hint);
        Qt3DCore::QTransform* pivotTransform = new Qt3DCore::QTransform();
        pivot->addComponent(pivotTransform);
        
        pEntity->setParent(pivot);
        pTransform->setTranslation(QVector3D(0.9f, 0.0f, 0.0f));
        
        QPropertyAnimation* orbitAnim = new QPropertyAnimation(pivotTransform, "rotationY");
        orbitAnim->setStartValue(0.0f);
        orbitAnim->setEndValue(360.0f);
        int duration = 1500 + QRandomGenerator::global()->bounded(1500);
        orbitAnim->setDuration(duration);
        orbitAnim->setLoopCount(-1);
        
        float startAngle = (360.0f / particleCount) * i;
        pivotTransform->setRotationY(startAngle);
        orbitAnim->setStartValue(startAngle);
        orbitAnim->setEndValue(startAngle + 360.0f);
        
        float tiltX = QRandomGenerator::global()->bounded(60) - 30;
        float tiltZ = QRandomGenerator::global()->bounded(60) - 30;
        QQuaternion tilt = QQuaternion::fromEulerAngles(tiltX, 0, tiltZ);
        
        Qt3DCore::QEntity* tiltRoot = new Qt3DCore::QEntity(m_particlesRoot_hint);
        Qt3DCore::QTransform* tiltTransform = new Qt3DCore::QTransform();
        tiltTransform->setRotation(tilt);
        tiltRoot->addComponent(tiltTransform);
        
        pivot->setParent(tiltRoot);

        orbitAnim->start();
        m_particleAnimations_hint.push_back(orbitAnim);
    }
}

void Gemstone::clearHintEffects() {
    if (m_particlesRoot_hint) {
        m_particlesRoot_hint->setParent((Qt3DCore::QNode*)nullptr);
        delete m_particlesRoot_hint;
        m_particlesRoot_hint = nullptr;
    }

    m_particleEntities_hint.clear();
    for (auto anim : m_particleAnimations_hint) {
        delete anim;
    }
    m_particleAnimations_hint.clear();
}

void Gemstone::clearSpecialEffects() {
    if (m_haloEntity) {
        m_haloEntity->setParent((Qt3DCore::QNode*)nullptr);
        delete m_haloEntity;
        m_haloEntity = nullptr;
    }
    if (m_haloScaleAnimation) {
        delete m_haloScaleAnimation;
        m_haloScaleAnimation = nullptr;
    }

    if (m_particlesRoot) {
        m_particlesRoot->setParent((Qt3DCore::QNode*)nullptr);
        delete m_particlesRoot;
        m_particlesRoot = nullptr;
    }

    m_particleEntities.clear();
    for (auto anim : m_particleAnimations) {
        delete anim;
    }
    m_particleAnimations.clear();
}

// ============================================================================
// 金币宝石实现
// ============================================================================

bool Gemstone::isCoinGem() const {
    return m_isCoinGem;
}

void Gemstone::setCoinGem(bool isCoin) {
    m_isCoinGem = isCoin;
    // 如果设置为金币宝石，显示金币图标
    if (m_isCoinGem) {
        setupCoinIndicator();
    } else {
        clearCoinIndicator();
    }
}

int Gemstone::getCoinValue() const {
    return m_coinValue;
}

void Gemstone::setCoinValue(int value) {
    m_coinValue = value;
}

// ============================================================================
// 金币图标实现
// ============================================================================

void Gemstone::setupCoinIndicator() {
    // 清除旧的金币图标（如果有）
    clearCoinIndicator();

    // 创建金币根实体
    m_coinIndicator = new Qt3DCore::QEntity(this);

    // === 主金币（圆柱体）===
    auto* coinEntity = new Qt3DCore::QEntity(m_coinIndicator);

    auto* coinMesh = new Qt3DExtras::QCylinderMesh(coinEntity);
    coinMesh->setRadius(0.25f);  // 半径稍小
    coinMesh->setLength(0.08f);  // 厚度（很薄，像硬币）
    coinMesh->setRings(2);
    coinMesh->setSlices(24);     // 更圆滑

    // 金色材质 - 更亮的金色
    auto* coinMaterial = new Qt3DExtras::QPhongMaterial(coinEntity);
    coinMaterial->setDiffuse(QColor(255, 223, 0));      // 亮金色
    coinMaterial->setAmbient(QColor(200, 160, 0));      // 深金色环境光
    coinMaterial->setSpecular(QColor(255, 255, 220));   // 亮黄色高光
    coinMaterial->setShininess(250.0f);                  // 高光泽度

    auto* coinTransform = new Qt3DCore::QTransform(coinEntity);
    coinTransform->setRotationX(0.0f);

    coinEntity->addComponent(coinMesh);
    coinEntity->addComponent(coinMaterial);
    coinEntity->addComponent(coinTransform);

    // === 数字显示（使用小球体组成数字形状）===
    // 创建数字实体容器
    auto* numberEntity = new Qt3DCore::QEntity(m_coinIndicator);
    auto* numberTransform = new Qt3DCore::QTransform(numberEntity);
    numberTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.06f)); // 放在金币前面
    numberTransform->setScale(0.4f); // 缩小数字
    numberEntity->addComponent(numberTransform);

    // 根据金币价值显示不同数量的小点
    float dotRadius = 0.08f;
    QColor numberColor(255, 100, 100); // 红色数字，醒目

    // 根据数值显示点阵
    std::vector<QVector3D> dotPositions;
    switch (m_coinValue) {
        case 1:
            dotPositions.push_back(QVector3D(0.0f, 0.0f, 0.0f));
            break;
        case 2:
            dotPositions.push_back(QVector3D(-0.12f, 0.0f, 0.0f));
            dotPositions.push_back(QVector3D(0.12f, 0.0f, 0.0f));
            break;
        case 3:
            dotPositions.push_back(QVector3D(-0.15f, 0.0f, 0.0f));
            dotPositions.push_back(QVector3D(0.0f, 0.0f, 0.0f));
            dotPositions.push_back(QVector3D(0.15f, 0.0f, 0.0f));
            break;
        case 4:
            dotPositions.push_back(QVector3D(-0.12f, 0.12f, 0.0f));
            dotPositions.push_back(QVector3D(0.12f, 0.12f, 0.0f));
            dotPositions.push_back(QVector3D(-0.12f, -0.12f, 0.0f));
            dotPositions.push_back(QVector3D(0.12f, -0.12f, 0.0f));
            break;
        case 5:
            dotPositions.push_back(QVector3D(-0.12f, 0.12f, 0.0f));
            dotPositions.push_back(QVector3D(0.12f, 0.12f, 0.0f));
            dotPositions.push_back(QVector3D(0.0f, 0.0f, 0.0f));
            dotPositions.push_back(QVector3D(-0.12f, -0.12f, 0.0f));
            dotPositions.push_back(QVector3D(0.12f, -0.12f, 0.0f));
            break;
        default:
            dotPositions.push_back(QVector3D(0.0f, 0.0f, 0.0f));
            break;
    }

    // 创建数字点
    for (const auto& pos : dotPositions) {
        auto* dotEntity = new Qt3DCore::QEntity(numberEntity);

        auto* dotMesh = new Qt3DExtras::QSphereMesh(dotEntity);
        dotMesh->setRadius(dotRadius);
        dotMesh->setRings(8);
        dotMesh->setSlices(8);

        auto* dotMaterial = new Qt3DExtras::QPhongMaterial(dotEntity);
        dotMaterial->setDiffuse(numberColor);
        dotMaterial->setAmbient(numberColor.darker(120));
        dotMaterial->setSpecular(QColor(255, 200, 200));
        dotMaterial->setShininess(100.0f);

        auto* dotTransform = new Qt3DCore::QTransform(dotEntity);
        dotTransform->setTranslation(pos);

        dotEntity->addComponent(dotMesh);
        dotEntity->addComponent(dotMaterial);
        dotEntity->addComponent(dotTransform);
    }

    // === 金币整体位置变换 ===
    auto* coinIndicatorTransform = new Qt3DCore::QTransform(m_coinIndicator);
    coinIndicatorTransform->setTranslation(QVector3D(0.0f, 0.65f, 0.0f)); // 更贴近宝石
    coinIndicatorTransform->setScale(0.8f);
    coinIndicatorTransform->setRotationX(15.0f); // 稍微倾斜，更有立体感
    m_coinIndicator->addComponent(coinIndicatorTransform);

    // === 添加旋转动画 ===
    m_coinRotationAnimation = new QPropertyAnimation(coinIndicatorTransform, "rotationY");
    m_coinRotationAnimation->setStartValue(15.0f);      // 从15度开始
    m_coinRotationAnimation->setEndValue(375.0f);       // 转到375度（360+15）
    m_coinRotationAnimation->setDuration(3000);         // 3秒转一圈，更慢更优雅
    m_coinRotationAnimation->setLoopCount(-1);
    m_coinRotationAnimation->setEasingCurve(QEasingCurve::Linear);
    m_coinRotationAnimation->start();

    qDebug() << "[Gemstone] Coin indicator created with value:" << m_coinValue;
}

void Gemstone::clearCoinIndicator() {
    if (m_coinIndicator) {
        // 停止动画
        if (m_coinRotationAnimation) {
            m_coinRotationAnimation->stop();
            delete m_coinRotationAnimation;
            m_coinRotationAnimation = nullptr;
        }

        // 删除金币实体
        m_coinIndicator->setParent((Qt3DCore::QNode*)nullptr);
        delete m_coinIndicator;
        m_coinIndicator = nullptr;

        qDebug() << "[Gemstone] Coin indicator cleared";
    }
}
