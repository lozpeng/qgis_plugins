#pragma once

#include <QDialog>
#include <QCombobox>

#include <qgisinterface.h>

#include "ui_qgsDataInqueryDialogBase.h"

struct sDataInqueryParams
{
	QString srcLyrIdx;
	QString srcFldName;
	QString tarLyrIdx;
	QString tarFldName;

};
//!数据空间查询获取工具
class qgsDataInqueryDialog : public QDialog, public Ui::qgsDataInqueryDialogBase
{
	Q_OBJECT
public:
	qgsDataInqueryDialog(QgisInterface* qgsInterface);

	~qgsDataInqueryDialog();
	bool getParams(sDataInqueryParams& params);
private:
	QgisInterface* mQgsInterface;  //qgis入口

	
protected slots:
	void on_mComboTargetLyr_currentIndexChanged(int index);

	void on_mComboSrcLyr_currentIndexChanged(int index);

	void on_pbOk_clicked(bool checked);

	void on_pbCancel_clicked(bool checked);
};

