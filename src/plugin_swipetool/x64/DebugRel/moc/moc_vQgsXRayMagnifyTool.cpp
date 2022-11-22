/****************************************************************************
** Meta object code from reading C++ file 'vQgsXRayMagnifyTool.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/extools/vQgsXRayMagnifyTool.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'vQgsXRayMagnifyTool.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_vQgsXRayMagnifyTool_t {
    QByteArrayData data[7];
    char stringdata0[78];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_vQgsXRayMagnifyTool_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_vQgsXRayMagnifyTool_t qt_meta_stringdata_vQgsXRayMagnifyTool = {
    {
QT_MOC_LITERAL(0, 0, 19), // "vQgsXRayMagnifyTool"
QT_MOC_LITERAL(1, 20, 6), // "setMap"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 7), // "disable"
QT_MOC_LITERAL(4, 36, 22), // "on_currentLayerChanged"
QT_MOC_LITERAL(5, 59, 12), // "QgsMapLayer*"
QT_MOC_LITERAL(6, 72, 5) // "layer"

    },
    "vQgsXRayMagnifyTool\0setMap\0\0disable\0"
    "on_currentLayerChanged\0QgsMapLayer*\0"
    "layer"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_vQgsXRayMagnifyTool[] = {

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
       1,    0,   29,    2, 0x08 /* Private */,
       3,    0,   30,    2, 0x08 /* Private */,
       4,    1,   31,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,

       0        // eod
};

void vQgsXRayMagnifyTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<vQgsXRayMagnifyTool *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setMap(); break;
        case 1: _t->disable(); break;
        case 2: _t->on_currentLayerChanged((*reinterpret_cast< QgsMapLayer*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QgsMapLayer* >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject vQgsXRayMagnifyTool::staticMetaObject = { {
    QMetaObject::SuperData::link<QgsMapTool::staticMetaObject>(),
    qt_meta_stringdata_vQgsXRayMagnifyTool.data,
    qt_meta_data_vQgsXRayMagnifyTool,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *vQgsXRayMagnifyTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *vQgsXRayMagnifyTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_vQgsXRayMagnifyTool.stringdata0))
        return static_cast<void*>(this);
    return QgsMapTool::qt_metacast(_clname);
}

int vQgsXRayMagnifyTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QgsMapTool::qt_metacall(_c, _id, _a);
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
