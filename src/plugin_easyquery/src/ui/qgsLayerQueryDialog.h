#pragma once

#include <QObject>
#include <QDialog>
#include "ui_qgsLayerQueryBaseDialog.h"
//! ͼ���ֶβ�ѯ�Ի���
class qgsLayerQueryDialog  : public QDialog,private Ui::qgsLayerQueryBaseDialog
{
	Q_OBJECT

public:
	qgsLayerQueryDialog(QWidget* parent);
	~qgsLayerQueryDialog();
};
