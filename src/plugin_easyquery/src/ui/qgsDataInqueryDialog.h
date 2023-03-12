#pragma once

#include <QDialog>
#include <QCombobox>

#include <qgisinterface.h>

#include "ui_qgsDataInqueryDialogBase.h"

//!数据空间查询获取工具
class qgsDataInqueryDialog : public QDialog, public Ui::qgsDataInqueryDialogBase
{
	Q_OBJECT
public:
	qgsDataInqueryDialog(QgisInterface* qgsInterface);

	~qgsDataInqueryDialog();

private:
	QgisInterface* mQgsInterface;  //qgis入口

private://functions
	/// <summary>
	/// 
	/// </summary>
	/// <param name="exLyrId">需要排除的图层名称</param>
	void loadLayerNames(QComboBox*,QString exLyrId);
protected slots:
	void on_mComboTargetLyr_currentIndexChanged(int index);

	void on_mComboSrcLyr_currentIndexChanged(int index);
};

