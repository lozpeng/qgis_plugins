#pragma once
#include <qgisinterface.h>

#include "base/QgsBaseActionFilter.h"

#include "qgsSamplingParamDialog.h"
/**
	动物监测区抽样单元格生成
*/
class qgsAnimalSamplGridAction :public QgsBaseActionFilter
{
	Q_OBJECT

public:
	qgsAnimalSamplGridAction(QgisInterface* qgsInterface);
	virtual ~qgsAnimalSamplGridAction();
	//!计算
	int compute();
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();
private:
	qgsSamplingParamDialog* mParamDialog; //参数对话框
	QgisInterface* mQgsInterface;
	qgsSampleParams mParams;

private slots:
	//!
	void on_currentlayer_changed(QgsMapLayer* lyr);
};