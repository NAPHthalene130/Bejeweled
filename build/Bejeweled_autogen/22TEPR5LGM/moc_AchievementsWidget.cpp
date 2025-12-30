/****************************************************************************
** Meta object code from reading C++ file 'AchievementsWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/game/gameWidgets/AchievementsWidget.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AchievementsWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSAchievementsBackgroundDecorationENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSAchievementsBackgroundDecorationENDCLASS = QtMocHelpers::stringData(
    "AchievementsBackgroundDecoration",
    "updateAnimation",
    ""
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSAchievementsBackgroundDecorationENDCLASS_t {
    uint offsetsAndSizes[6];
    char stringdata0[33];
    char stringdata1[16];
    char stringdata2[1];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSAchievementsBackgroundDecorationENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSAchievementsBackgroundDecorationENDCLASS_t qt_meta_stringdata_CLASSAchievementsBackgroundDecorationENDCLASS = {
    {
        QT_MOC_LITERAL(0, 32),  // "AchievementsBackgroundDecoration"
        QT_MOC_LITERAL(33, 15),  // "updateAnimation"
        QT_MOC_LITERAL(49, 0)   // ""
    },
    "AchievementsBackgroundDecoration",
    "updateAnimation",
    ""
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSAchievementsBackgroundDecorationENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   20,    2, 0x08,    1 /* Private */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject AchievementsBackgroundDecoration::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSAchievementsBackgroundDecorationENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSAchievementsBackgroundDecorationENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSAchievementsBackgroundDecorationENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<AchievementsBackgroundDecoration, std::true_type>,
        // method 'updateAnimation'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void AchievementsBackgroundDecoration::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AchievementsBackgroundDecoration *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->updateAnimation(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *AchievementsBackgroundDecoration::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AchievementsBackgroundDecoration::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSAchievementsBackgroundDecorationENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int AchievementsBackgroundDecoration::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSAchievementsWidgetENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSAchievementsWidgetENDCLASS = QtMocHelpers::stringData(
    "AchievementsWidget",
    "backToMenu",
    "",
    "onPrevPage",
    "onNextPage",
    "onBackClicked"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSAchievementsWidgetENDCLASS_t {
    uint offsetsAndSizes[12];
    char stringdata0[19];
    char stringdata1[11];
    char stringdata2[1];
    char stringdata3[11];
    char stringdata4[11];
    char stringdata5[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSAchievementsWidgetENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSAchievementsWidgetENDCLASS_t qt_meta_stringdata_CLASSAchievementsWidgetENDCLASS = {
    {
        QT_MOC_LITERAL(0, 18),  // "AchievementsWidget"
        QT_MOC_LITERAL(19, 10),  // "backToMenu"
        QT_MOC_LITERAL(30, 0),  // ""
        QT_MOC_LITERAL(31, 10),  // "onPrevPage"
        QT_MOC_LITERAL(42, 10),  // "onNextPage"
        QT_MOC_LITERAL(53, 13)   // "onBackClicked"
    },
    "AchievementsWidget",
    "backToMenu",
    "",
    "onPrevPage",
    "onNextPage",
    "onBackClicked"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSAchievementsWidgetENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   38,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    0,   39,    2, 0x08,    2 /* Private */,
       4,    0,   40,    2, 0x08,    3 /* Private */,
       5,    0,   41,    2, 0x08,    4 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject AchievementsWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSAchievementsWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSAchievementsWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSAchievementsWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<AchievementsWidget, std::true_type>,
        // method 'backToMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPrevPage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onNextPage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBackClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void AchievementsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AchievementsWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->backToMenu(); break;
        case 1: _t->onPrevPage(); break;
        case 2: _t->onNextPage(); break;
        case 3: _t->onBackClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AchievementsWidget::*)();
            if (_t _q_method = &AchievementsWidget::backToMenu; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
    (void)_a;
}

const QMetaObject *AchievementsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AchievementsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSAchievementsWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int AchievementsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void AchievementsWidget::backToMenu()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
