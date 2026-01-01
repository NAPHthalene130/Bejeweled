#ifndef GEMSTONE_H
#define GEMSTONE_H

#include <string>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QMesh>
#include <QPropertyAnimation>
#include <QUrl>
#include <QFileInfo>
#include <QMap>
#include <QObject>

/**
 * @brief 宝石风格枚举
 */
enum class GemstoneStyle {
    Builtin = 0,    // 内置几何体
    Gemstones,      // 宝石模型
    Planets,        // 八大行星
    FastFood,       // 美食风格
    // 预留扩展
    Custom1,        // 自定义风格1
    Custom2         // 自定义风格2
};

/**
 * @brief 宝石模型管理器 - 单例模式
 * 负责管理宝石模型路径和风格配置
 */
class GemstoneModelManager : public QObject {
    Q_OBJECT
    
public:
    static GemstoneModelManager& instance();
    
    // ==================== 风格管理 ====================
    
    // 设置当前风格
    void setCurrentStyle(GemstoneStyle style);
    void setCurrentStyleByName(const QString& styleName);
    GemstoneStyle getCurrentStyle() const;
    QString getCurrentStyleName() const;
    
    // 获取所有可用风格
    QStringList getAvailableStyles() const;
    
    // 风格名称转换
    static QString styleToName(GemstoneStyle style);
    static GemstoneStyle nameToStyle(const QString& name);
    
    // ==================== 模型路径管理 ====================
    
    // 设置资源根目录
    void setResourcesDirectory(const QString& path);
    QString getResourcesDirectory() const;
    
    // 获取特定风格的模型目录
    QString getStyleDirectory(GemstoneStyle style) const;
    
    // 获取特定类型宝石的模型路径（使用当前风格）
    QString getModelPath(int type) const;
    
    // 获取特定风格和类型的模型路径
    QString getModelPath(GemstoneStyle style, int type) const;
    
    // 检查是否存在外部模型
    bool hasExternalModel(int type) const;
    bool hasExternalModel(GemstoneStyle style, int type) const;
    
    // 获取当前风格支持的模型数量
    int getModelCount() const;
    
    // 刷新模型缓存
    void refreshModelCache();
    
    // 模型文件名模式
    void setModelFilePattern(const QString& pattern);
    QString getModelFilePattern() const;

signals:
    // 风格变化信号
    void styleChanged(GemstoneStyle newStyle);
    void modelsReloaded();

private:
    GemstoneModelManager();
    ~GemstoneModelManager() = default;
    GemstoneModelManager(const GemstoneModelManager&) = delete;
    GemstoneModelManager& operator=(const GemstoneModelManager&) = delete;
    
    GemstoneStyle m_currentStyle;
    QString m_resourcesDirectory;
    QString m_filePattern;
    
    // 每个风格的模型缓存: style -> (type -> path)
    QMap<GemstoneStyle, QMap<int, QString>> m_modelCache;
    
    // 风格目录名映射
    QMap<GemstoneStyle, QString> m_styleDirectories;
    
    void initStyleDirectories();
    void scanStyleDirectory(GemstoneStyle style);
    void scanAllStyles();
};

/**
 * @brief 宝石实体类
 * 支持多种风格：
 * 1. 内置基础几何体
 * 2. 外部3D模型（宝石、行星、美食等）
 */
class Gemstone : public Qt3DCore::QEntity {
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param type 宝石类型（0-7）
     * @param style 宝石风格字符串（兼容旧代码）
     * @param parent 父节点
     */
    Gemstone(int type, std::string style, Qt3DCore::QNode* parent = nullptr);
    ~Gemstone();

    int getType() const;
    void setType(int type);

    std::string getStyle() const;
    void setStyle(const std::string& style);

    Qt3DCore::QTransform* transform() const;
    bool isSpecial() const;
    bool isHint() const;
    void setSpecial(bool special);
    void setHint(bool hint);
    
    // 检查是否使用外部模型
    bool isUsingExternalModel() const;
    
    // 重新加载模型（风格变化时调用）
    void reloadModel();

    bool getCanBeChosen() const;
    void setCanBeChosen(bool can);

    // 金币宝石相关
    bool isCoinGem() const;
    void setCoinGem(bool isCoin);
    int getCoinValue() const;
    void setCoinValue(int value);

signals:
    void clicked(Gemstone* self);
    void pickEvent(const QString& info);
    void modelLoaded(bool success);

private slots:
    // 响应全局风格变化
    void onGlobalStyleChanged(GemstoneStyle newStyle);

private:
    int type;
    std::string style;
    bool special = false;
    bool hint = false;
    bool m_usingExternalModel = false;
    bool canBeChosen = true;

    // 金币宝石属性
    bool m_isCoinGem = false;
    int m_coinValue = 0;

    Qt3DCore::QTransform* m_transform;
    Qt3DExtras::QPhongMaterial* m_material;
    Qt3DRender::QGeometryRenderer* m_mesh;
    Qt3DRender::QMesh* m_externalMesh;
    Qt3DRender::QObjectPicker* m_picker;
    QPropertyAnimation* m_rotationAnimation;

    // Special effects components
    Qt3DCore::QEntity* m_haloEntity = nullptr;
    QPropertyAnimation* m_haloScaleAnimation = nullptr;
    Qt3DCore::QEntity* m_particlesRoot = nullptr;
    std::vector<Qt3DCore::QEntity*> m_particleEntities;
    std::vector<QPropertyAnimation*> m_particleAnimations;

    // Hint effects componets
    Qt3DCore::QEntity* m_particlesRoot_hint = nullptr;
    std::vector<Qt3DCore::QEntity*> m_particleEntities_hint;
    std::vector<QPropertyAnimation*> m_particleAnimations_hint;

    // 金币图标
    Qt3DCore::QEntity* m_coinIndicator = nullptr;
    QPropertyAnimation* m_coinRotationAnimation = nullptr;

    void updateAppearance();
    void setupMesh();
    void setupExternalMesh();
    void setupBuiltinMesh();
    void setupMaterial();
    void updateSpecialEffects();
    void clearSpecialEffects();
    void updateHintEffects();
    void clearHintEffects();

    // 金币图标相关
    void setupCoinIndicator();
    void clearCoinIndicator();
    
    // 获取宝石颜色（根据当前风格）
    QColor getGemColor() const;
    
    // 获取内置几何体颜色
    QColor getBuiltinColor() const;
    
    // 获取宝石风格颜色
    QColor getGemstoneColor() const;
    
    // 获取行星风格颜色
    QColor getPlanetColor() const;
    
    // 获取美食风格颜色
    QColor getFastFoodColor() const;
};

#endif // GEMSTONE_H
