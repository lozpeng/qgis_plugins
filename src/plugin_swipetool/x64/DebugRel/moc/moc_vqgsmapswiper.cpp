/****************************************************************************
** Meta object code from reading C++ file 'vqgsmapswiper.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/extools/vqgsmapswiper.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'vqgsmapswiper.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_vQgsMapSwiper_t {
    QByteArrayData data[9];
    char stringdata0[104];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_vQgsMapSwiper_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_vQgsMapSwiper_t qt_meta_stringdata_vQgsMapSwiper = {
    {
QT_MOC_LITERAL(0, 0, 13), // "vQgsMapSwiper"
QT_MOC_LITERAL(1, 14, 18), // "on_ation_triggered"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 7), // "checked"
QT_MOC_LITERAL(4, 42, 19), // "on_xation_triggered"
QT_MOC_LITERAL(5, 62, 13), // "on_mapToolSet"
QT_MOC_LITERAL(6, 76, 11), // "QgsMapTool*"
QT_MOC_LITERAL(7, 88, 7), // "newTool"
QT_MOC_LITERAL(8, 96, 7) // "oldTool"

    },
    "vQgsMapSwiper\0on_ation_triggered\0\0"
    "checked\0on_xation_triggered\0on_mapToolSet\0"
    "QgsMapTool*\0newTool\0oldTool"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_vQgsMapSwiper[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x08 /* Private */,
       4,    1,   32,    2, 0x08 /* Private */,
       5,    2,   35,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    7,    8,

       0        // eod
};

void vQgsMapSwiper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<vQgsMapSwiper *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_ation_triggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->on_xation_triggered((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->on_mapToolSet((*reinterpret_cast< QgsMapTool*(*)>(_a[1])),(*reinterpret_cast< QgsMapTool*(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 1:
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QgsMapTool* >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject vQgsMapSwiper::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_vQgsMapSwiper.data,
    qt_meta_data_vQgsMapSwiper,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *vQgsMapSwiper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *vQgsMapSwiper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_vQgsMapSwiper.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int vQgsMapSwiper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
