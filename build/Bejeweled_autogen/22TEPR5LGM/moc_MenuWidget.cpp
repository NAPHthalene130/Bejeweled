/****************************************************************************
** Meta object code from reading C++ file 'MenuWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/game/gameWidgets/MenuWidget.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MenuWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSMenuWidgetENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSMenuWidgetENDCLASS = QtMocHelpers::stringData(
    "MenuWidget",
    "startGame",
    "",
    "openStore",
    "openAchievements",
    "openLeaderboard",
    "openSettings",
    "playMenuButtonClicked",
    "storeButtonClicked",
    "achievementsButtonClicked",
    "ranklistButtonClicked",
    "settingsButtonClicked",
    "exitButtonClicked"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSMenuWidgetENDCLASS_t {
    uint offsetsAndSizes[26];
    char stringdata0[11];
    char stringdata1[10];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[17];
    char stringdata5[16];
    char stringdata6[13];
    char stringdata7[22];
    char stringdata8[19];
    char stringdata9[26];
    char stringdata10[22];
    char stringdata11[22];
    char stringdata12[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSMenuWidgetENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSMenuWidgetENDCLASS_t qt_meta_stringdata_CLASSMenuWidgetENDCLASS = {
    {
        QT_MOC_LITERAL(0, 10),  // "MenuWidget"
        QT_MOC_LITERAL(11, 9),  // "startGame"
        QT_MOC_LITERAL(21, 0),  // ""
        QT_MOC_LITERAL(22, 9),  // "openStore"
        QT_MOC_LITERAL(32, 16),  // "openAchievements"
        QT_MOC_LITERAL(49, 15),  // "openLeaderboard"
        QT_MOC_LITERAL(65, 12),  // "openSettings"
        QT_MOC_LITERAL(78, 21),  // "playMenuButtonClicked"
        QT_MOC_LITERAL(100, 18),  // "storeButtonClicked"
        QT_MOC_LITERAL(119, 25),  // "achievementsButtonClicked"
        QT_MOC_LITERAL(145, 21),  // "ranklistButtonClicked"
        QT_MOC_LITERAL(167, 21),  // "settingsButtonClicked"
        QT_MOC_LITERAL(189, 17)   // "exitButtonClicked"
    },
    "MenuWidget",
    "startGame",
    "",
    "openStore",
    "openAchievements",
    "openLeaderboard",
    "openSettings",
    "playMenuButtonClicked",
    "storeButtonClicked",
    "achievementsButtonClicked",
    "ranklistButtonClicked",
    "settingsButtonClicked",
    "exitButtonClicked"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMenuWidgetENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x06,    1 /* Public */,
       3,    0,   81,    2, 0x06,    2 /* Public */,
       4,    0,   82,    2, 0x06,    3 /* Public */,
       5,    0,   83,    2, 0x06,    4 /* Public */,
       6,    0,   84,    2, 0x06,    5 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       7,    0,   85,    2, 0x08,    6 /* Private */,
       8,    0,   86,    2, 0x08,    7 /* Private */,
       9,    0,   87,    2, 0x08,    8 /* Private */,
      10,    0,   88,    2, 0x08,    9 /* Private */,
      11,    0,   89,    2, 0x08,   10 /* Private */,
      12,    0,   90,    2, 0x08,   11 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MenuWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSMenuWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMenuWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMenuWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MenuWidget, std::true_type>,
        // method 'startGame'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openStore'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openAchievements'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openLeaderboard'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openSettings'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'playMenuButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'storeButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'achievementsButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'ranklistButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'settingsButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'exitButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MenuWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MenuWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->startGame(); break;
        case 1: _t->openStore(); break;
        case 2: _t->openAchievements(); break;
        case 3: _t->openLeaderboard(); break;
        case 4: _t->openSettings(); break;
        case 5: _t->playMenuButtonClicked(); break;
        case 6: _t->storeButtonClicked(); break;
        case 7: _t->achievementsButtonClicked(); break;
        case 8: _t->ranklistButtonClicked(); break;
        case 9: _t->settingsButtonClicked(); break;
        case 10: _t->exitButtonClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MenuWidget::*)();
            if (_t _q_method = &MenuWidget::startGame; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MenuWidget::*)();
            if (_t _q_method = &MenuWidget::openStore; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MenuWidget::*)();
            if (_t _q_method = &MenuWidget::openAchievements; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MenuWidget::*)();
            if (_t _q_method = &MenuWidget::openLeaderboard; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (MenuWidget::*)();
            if (_t _q_method = &MenuWidget::openSettings; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
    (void)_a;
}

const QMetaObject *MenuWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MenuWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMenuWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MenuWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void MenuWidget::startGame()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MenuWidget::openStore()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void MenuWidget::openAchievements()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void MenuWidget::openLeaderboard()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void MenuWidget::openSettings()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
