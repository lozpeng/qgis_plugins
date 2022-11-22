/****************************************************************************
** Meta object code from reading C++ file 'qgsVectorSplitAction.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/editor/split/qgsVectorSplitAction.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qgsVectorSplitAction.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_qgsVectorSplitAction_t {
    QByteArrayData data[4];
    char stringdata0[63];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_qgsVectorSplitAction_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_qgsVectorSplitAction_t qt_meta_stringdata_qgsVectorSplitAction = {
    {
QT_MOC_LITERAL(0, 0, 20), // "qgsVectorSplitAction"
QT_MOC_LITERAL(1, 21, 16), // "on_layer_editing"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 23) // "on_layer_editingStopped"

    },
    "qgsVectorSplitAction\0on_layer_editing\0"
    "\0on_layer_editingStopped"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_qgsVectorSplitAction[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x08 /* Private */,
       3,    0,   25,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void qgsVectorSplitAction::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<qgsVectorSplitAction *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_layer_editing(); break;
        case 1: _t->on_layer_editingStopped(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject qgsVectorSplitAction::staticMetaObject = { {
    QMetaObject::SuperData::link<QgsBaseActionFilter::staticMetaObject>(),
    qt_meta_stringdata_qgsVectorSplitAction.data,
    qt_meta_data_qgsVectorSplitAction,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *qgsVectorSplitAction::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *qgsVectorSplitAction::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_qgsVectorSplitAction.stringdata0))
        return static_cast<void*>(this);
    return QgsBaseActionFilter::qt_metacast(_clname);
}

int qgsVectorSplitAction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QgsBaseActionFilter::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
