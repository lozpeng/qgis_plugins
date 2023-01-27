#pragma once
#include <qgisinterface.h>

#include "base/QgsBaseActionFilter.h"

//!评估参数
typedef struct qgsEvalParams
{



}evalParams;
//!抽样评估，分析计算抽样强度
class qgsSamplingEvalAction :public QgsBaseActionFilter
{
	Q_OBJECT

public:
	qgsSamplingEvalAction(QgisInterface* qgsInterface);
	virtual ~qgsSamplingEvalAction();
	//!计算
	int compute();
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();
private:
	//qgsSamplingParamDialog* mParamDialog; //参数对话框
	QgisInterface* mQgsInterface;
	qgsEvalParams mParams;

private slots:
	//!
	void on_currentlayer_changed(QgsMapLayer* lyr);


};

