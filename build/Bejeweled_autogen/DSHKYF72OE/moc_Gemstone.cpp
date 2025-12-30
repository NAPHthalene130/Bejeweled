/****************************************************************************
** Meta object code from reading C++ file 'Gemstone.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/game/components/Gemstone.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Gemstone.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSGemstoneModelManagerENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSGemstoneModelManagerENDCLASS = QtMocHelpers::stringData(
    "GemstoneModelManager",
    "styleChanged",
    "",
    "GemstoneStyle",
    "newStyle",
    "modelsReloaded"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSGemstoneModelManagerENDCLASS_t {
    uint offsetsAndSizes[12];
    char stringdata0[21];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[9];
    char stringdata5[15];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSGemstoneModelManagerENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSGemstoneModelManagerENDCLASS_t qt_meta_stringdata_CLASSGemstoneModelManagerENDCLASS = {
    {
        QT_MOC_LITERAL(0, 20),  // "GemstoneModelManager"
        QT_MOC_LITERAL(21, 12),  // "styleChanged"
        QT_MOC_LITERAL(34, 0),  // ""
        QT_MOC_LITERAL(35, 13),  // "GemstoneStyle"
        QT_MOC_LITERAL(49, 8),  // "newStyle"
        QT_MOC_LITERAL(58, 14)   // "modelsReloaded"
    },
    "GemstoneModelManager",
    "styleChanged",
    "",
    "GemstoneStyle",
    "newStyle",
    "modelsReloaded"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSGemstoneModelManagerENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   26,    2, 0x06,    1 /* Public */,
       5,    0,   29,    2, 0x06,    3 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject GemstoneModelManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSGemstoneModelManagerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSGemstoneModelManagerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSGemstoneModelManagerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<GemstoneModelManager, std::true_type>,
        // method 'styleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<GemstoneStyle, std::false_type>,
        // method 'modelsReloaded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void GemstoneModelManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GemstoneModelManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->styleChanged((*reinterpret_cast< std::add_pointer_t<GemstoneStyle>>(_a[1]))); break;
        case 1: _t->modelsReloaded(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GemstoneModelManager::*)(GemstoneStyle );
            if (_t _q_method = &GemstoneModelManager::styleChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GemstoneModelManager::*)();
            if (_t _q_method = &GemstoneModelManager::modelsReloaded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject *GemstoneModelManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GemstoneModelManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSGemstoneModelManagerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GemstoneModelManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void GemstoneModelManager::styleChanged(GemstoneStyle _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GemstoneModelManager::modelsReloaded()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSGemstoneENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSGemstoneENDCLASS = QtMocHelpers::stringData(
    "Gemstone",
    "clicked",
    "",
    "Gemstone*",
    "self",
    "pickEvent",
    "info",
    "modelLoaded",
    "success",
    "onGlobalStyleChanged",
    "GemstoneStyle",
    "newStyle"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSGemstoneENDCLASS_t {
    uint offsetsAndSizes[24];
    char stringdata0[9];
    char stringdata1[8];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[5];
    char stringdata5[10];
    char stringdata6[5];
    char stringdata7[12];
    char stringdata8[8];
    char stringdata9[21];
    char stringdata10[14];
    char stringdata11[9];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSGemstoneENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSGemstoneENDCLASS_t qt_meta_stringdata_CLASSGemstoneENDCLASS = {
    {
        QT_MOC_LITERAL(0, 8),  // "Gemstone"
        QT_MOC_LITERAL(9, 7),  // "clicked"
        QT_MOC_LITERAL(17, 0),  // ""
        QT_MOC_LITERAL(18, 9),  // "Gemstone*"
        QT_MOC_LITERAL(28, 4),  // "self"
        QT_MOC_LITERAL(33, 9),  // "pickEvent"
        QT_MOC_LITERAL(43, 4),  // "info"
        QT_MOC_LITERAL(48, 11),  // "modelLoaded"
        QT_MOC_LITERAL(60, 7),  // "success"
        QT_MOC_LITERAL(68, 20),  // "onGlobalStyleChanged"
        QT_MOC_LITERAL(89, 13),  // "GemstoneStyle"
        QT_MOC_LITERAL(103, 8)   // "newStyle"
    },
    "Gemstone",
    "clicked",
    "",
    "Gemstone*",
    "self",
    "pickEvent",
    "info",
    "modelLoaded",
    "success",
    "onGlobalStyleChanged",
    "GemstoneStyle",
    "newStyle"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSGemstoneENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   38,    2, 0x06,    1 /* Public */,
       5,    1,   41,    2, 0x06,    3 /* Public */,
       7,    1,   44,    2, 0x06,    5 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       9,    1,   47,    2, 0x08,    7 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::Bool,    8,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 10,   11,

       0        // eod
};

Q_CONSTINIT const QMetaObject Gemstone::staticMetaObject = { {
    QMetaObject::SuperData::link<Qt3DCore::QEntity::staticMetaObject>(),
    qt_meta_stringdata_CLASSGemstoneENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSGemstoneENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSGemstoneENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Gemstone, std::true_type>,
        // method 'clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Gemstone *, std::false_type>,
        // method 'pickEvent'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'modelLoaded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onGlobalStyleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<GemstoneStyle, std::false_type>
    >,
    nullptr
} };

void Gemstone::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Gemstone *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->clicked((*reinterpret_cast< std::add_pointer_t<Gemstone*>>(_a[1]))); break;
        case 1: _t->pickEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->modelLoaded((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->onGlobalStyleChanged((*reinterpret_cast< std::add_pointer_t<GemstoneStyle>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< Gemstone* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Gemstone::*)(Gemstone * );
            if (_t _q_method = &Gemstone::clicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Gemstone::*)(const QString & );
            if (_t _q_method = &Gemstone::pickEvent; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Gemstone::*)(bool );
            if (_t _q_method = &Gemstone::modelLoaded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *Gemstone::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Gemstone::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSGemstoneENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return Qt3DCore::QEntity::qt_metacast(_clname);
}

int Gemstone::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Qt3DCore::QEntity::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void Gemstone::clicked(Gemstone * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Gemstone::pickEvent(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Gemstone::modelLoaded(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
