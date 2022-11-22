/****************************************************************************
** Meta object code from reading C++ file 'qgsVectorLayerSelectBaseDialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/editor/split/qgsVectorLayerSelectBaseDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qgsVectorLayerSelectBaseDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_qgsVectorLayerSelectBaseDialog_t {
    QByteArrayData data[5];
    char stringdata0[76];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_qgsVectorLayerSelectBaseDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_qgsVectorLayerSelectBaseDialog_t qt_meta_stringdata_qgsVectorLayerSelectBaseDialog = {
    {
QT_MOC_LITERAL(0, 0, 30), // "qgsVectorLayerSelectBaseDialog"
QT_MOC_LITERAL(1, 31, 15), // "on_pbOk_clicked"
QT_MOC_LITERAL(2, 47, 0), // ""
QT_MOC_LITERAL(3, 48, 7), // "checked"
QT_MOC_LITERAL(4, 56, 19) // "on_pbCancle_clicked"

    },
    "qgsVectorLayerSelectBaseDialog\0"
    "on_pbOk_clicked\0\0checked\0on_pbCancle_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_qgsVectorLayerSelectBaseDialog[] = {

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
       1,    1,   24,    2, 0x08 /* Private */,
       4,    1,   27,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Bool,    3,

       0        // eod
};

void qgsVectorLayerSelectBaseDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<qgsVectorLayerSelectBaseDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_pbOk_clicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->on_pbCancle_clicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject qgsVectorLayerSelectBaseDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_qgsVectorLayerSelectBaseDialog.data,
    qt_meta_data_qgsVectorLayerSelectBaseDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *qgsVectorLayerSelectBaseDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *qgsVectorLayerSelectBaseDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_qgsVectorLayerSelectBaseDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int qgsVectorLayerSelectBaseDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
