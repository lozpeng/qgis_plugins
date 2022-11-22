/********************************************************************************
** Form generated from reading UI file 'qgsLayerQueryBaseDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QGSLAYERQUERYBASEDIALOG_H
#define UI_QGSLAYERQUERYBASEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_qgsLayerQueryBaseDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *pbOk;
    QPushButton *pbCancel;

    void setupUi(QDialog *qgsLayerQueryBaseDialog)
    {
        if (qgsLayerQueryBaseDialog->objectName().isEmpty())
            qgsLayerQueryBaseDialog->setObjectName(QString::fromUtf8("qgsLayerQueryBaseDialog"));
        qgsLayerQueryBaseDialog->resize(624, 382);
        verticalLayout_2 = new QVBoxLayout(qgsLayerQueryBaseDialog);
        verticalLayout_2->setSpacing(3);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(3, 3, 3, 3);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        verticalLayout_2->addLayout(verticalLayout);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        pbOk = new QPushButton(qgsLayerQueryBaseDialog);
        pbOk->setObjectName(QString::fromUtf8("pbOk"));

        hboxLayout->addWidget(pbOk);

        pbCancel = new QPushButton(qgsLayerQueryBaseDialog);
        pbCancel->setObjectName(QString::fromUtf8("pbCancel"));

        hboxLayout->addWidget(pbCancel);


        verticalLayout_2->addLayout(hboxLayout);


        retranslateUi(qgsLayerQueryBaseDialog);

        QMetaObject::connectSlotsByName(qgsLayerQueryBaseDialog);
    } // setupUi

    void retranslateUi(QDialog *qgsLayerQueryBaseDialog)
    {
        qgsLayerQueryBaseDialog->setWindowTitle(QCoreApplication::translate("qgsLayerQueryBaseDialog", "Dialog", nullptr));
        pbOk->setText(QCoreApplication::translate("qgsLayerQueryBaseDialog", "\347\241\256\345\256\232", nullptr));
        pbCancel->setText(QCoreApplication::translate("qgsLayerQueryBaseDialog", "\345\217\226\346\266\210", nullptr));
    } // retranslateUi

};

namespace Ui {
    class qgsLayerQueryBaseDialog: public Ui_qgsLayerQueryBaseDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QGSLAYERQUERYBASEDIALOG_H
