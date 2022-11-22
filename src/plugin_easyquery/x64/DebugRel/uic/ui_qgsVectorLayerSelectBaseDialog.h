/********************************************************************************
** Form generated from reading UI file 'qgsVectorLayerSelectBaseDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QGSVECTORLAYERSELECTBASEDIALOG_H
#define UI_QGSVECTORLAYERSELECTBASEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_qgsVectorLayerSelectBaseDialogClass
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QComboBox *comboLayers;
    QWidget *widget;
    QPushButton *pbOk;
    QPushButton *pbCancel;

    void setupUi(QDialog *qgsVectorLayerSelectBaseDialogClass)
    {
        if (qgsVectorLayerSelectBaseDialogClass->objectName().isEmpty())
            qgsVectorLayerSelectBaseDialogClass->setObjectName(QString::fromUtf8("qgsVectorLayerSelectBaseDialogClass"));
        qgsVectorLayerSelectBaseDialogClass->resize(500, 300);
        qgsVectorLayerSelectBaseDialogClass->setMinimumSize(QSize(500, 300));
        qgsVectorLayerSelectBaseDialogClass->setMaximumSize(QSize(550, 350));
        qgsVectorLayerSelectBaseDialogClass->setModal(false);
        verticalLayout = new QVBoxLayout(qgsVectorLayerSelectBaseDialogClass);
        verticalLayout->setSpacing(3);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(3, 3, 3, 3);
        groupBox = new QGroupBox(qgsVectorLayerSelectBaseDialogClass);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        comboLayers = new QComboBox(groupBox);
        comboLayers->setObjectName(QString::fromUtf8("comboLayers"));

        verticalLayout_2->addWidget(comboLayers);


        verticalLayout->addWidget(groupBox);

        widget = new QWidget(qgsVectorLayerSelectBaseDialogClass);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setMinimumSize(QSize(295, 39));
        pbOk = new QPushButton(widget);
        pbOk->setObjectName(QString::fromUtf8("pbOk"));
        pbOk->setGeometry(QRect(50, 10, 75, 23));
        pbCancel = new QPushButton(widget);
        pbCancel->setObjectName(QString::fromUtf8("pbCancel"));
        pbCancel->setGeometry(QRect(150, 10, 75, 23));

        verticalLayout->addWidget(widget);


        retranslateUi(qgsVectorLayerSelectBaseDialogClass);

        QMetaObject::connectSlotsByName(qgsVectorLayerSelectBaseDialogClass);
    } // setupUi

    void retranslateUi(QDialog *qgsVectorLayerSelectBaseDialogClass)
    {
        qgsVectorLayerSelectBaseDialogClass->setWindowTitle(QCoreApplication::translate("qgsVectorLayerSelectBaseDialogClass", "\350\257\267\351\200\211\346\213\251\345\233\276\345\261\202", nullptr));
        groupBox->setTitle(QCoreApplication::translate("qgsVectorLayerSelectBaseDialogClass", "\345\233\276\345\261\202\351\200\211\346\213\251", nullptr));
        comboLayers->setPlaceholderText(QCoreApplication::translate("qgsVectorLayerSelectBaseDialogClass", "\350\257\267\351\200\211\346\213\251\345\210\207\345\211\262\345\233\276\345\261\202", nullptr));
        pbOk->setText(QCoreApplication::translate("qgsVectorLayerSelectBaseDialogClass", "\347\241\256\345\256\232", nullptr));
        pbCancel->setText(QCoreApplication::translate("qgsVectorLayerSelectBaseDialogClass", "\345\217\226\346\266\210", nullptr));
    } // retranslateUi

};

namespace Ui {
    class qgsVectorLayerSelectBaseDialogClass: public Ui_qgsVectorLayerSelectBaseDialogClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QGSVECTORLAYERSELECTBASEDIALOG_H
