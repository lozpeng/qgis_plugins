/********************************************************************************
** Form generated from reading UI file 'qgslayerfieldsbasedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QGSLAYERFIELDSBASEDIALOG_H
#define UI_QGSLAYERFIELDSBASEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_qgsLayerFieldsBaseDialogClass
{
public:
    QVBoxLayout *verticalLayout_3;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QCheckBox *chkAllFields;
    QListWidget *fieldListWidget;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pbOk;

    void setupUi(QDialog *qgsLayerFieldsBaseDialogClass)
    {
        if (qgsLayerFieldsBaseDialogClass->objectName().isEmpty())
            qgsLayerFieldsBaseDialogClass->setObjectName(QString::fromUtf8("qgsLayerFieldsBaseDialogClass"));
        qgsLayerFieldsBaseDialogClass->resize(234, 194);
        verticalLayout_3 = new QVBoxLayout(qgsLayerFieldsBaseDialogClass);
        verticalLayout_3->setSpacing(3);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(3, 3, 3, 3);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(qgsLayerFieldsBaseDialogClass);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        chkAllFields = new QCheckBox(qgsLayerFieldsBaseDialogClass);
        chkAllFields->setObjectName(QString::fromUtf8("chkAllFields"));

        verticalLayout->addWidget(chkAllFields);

        fieldListWidget = new QListWidget(qgsLayerFieldsBaseDialogClass);
        fieldListWidget->setObjectName(QString::fromUtf8("fieldListWidget"));

        verticalLayout->addWidget(fieldListWidget);


        verticalLayout_3->addLayout(verticalLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pbOk = new QPushButton(qgsLayerFieldsBaseDialogClass);
        pbOk->setObjectName(QString::fromUtf8("pbOk"));

        horizontalLayout->addWidget(pbOk);


        verticalLayout_3->addLayout(horizontalLayout);


        retranslateUi(qgsLayerFieldsBaseDialogClass);

        QMetaObject::connectSlotsByName(qgsLayerFieldsBaseDialogClass);
    } // setupUi

    void retranslateUi(QDialog *qgsLayerFieldsBaseDialogClass)
    {
        qgsLayerFieldsBaseDialogClass->setWindowTitle(QCoreApplication::translate("qgsLayerFieldsBaseDialogClass", "\345\233\276\345\261\202\345\255\227\346\256\265\351\200\211\346\213\251", nullptr));
        label->setText(QCoreApplication::translate("qgsLayerFieldsBaseDialogClass", "\351\200\211\346\213\251\345\255\227\346\256\265\357\274\232", nullptr));
        chkAllFields->setText(QCoreApplication::translate("qgsLayerFieldsBaseDialogClass", "\346\211\200\346\234\211\345\255\227\346\256\265...", nullptr));
        pbOk->setText(QCoreApplication::translate("qgsLayerFieldsBaseDialogClass", "\347\241\256  \345\256\232", nullptr));
    } // retranslateUi

};

namespace Ui {
    class qgsLayerFieldsBaseDialogClass: public Ui_qgsLayerFieldsBaseDialogClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QGSLAYERFIELDSBASEDIALOG_H
