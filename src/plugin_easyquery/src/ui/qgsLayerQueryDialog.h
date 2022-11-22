#pragma once

#include <QObject>
#include <QDialog>
#include "ui_qgsLayerQueryBaseDialog.h"
//! Í¼²ã×Ö¶Î²éÑ¯¶Ô»°¿ò
class qgsLayerQueryDialog  : public QDialog,private Ui::qgsLayerQueryBaseDialog
{
	Q_OBJECT

public:
	qgsLayerQueryDialog(QWidget* parent);
	~qgsLayerQueryDialog();
};
