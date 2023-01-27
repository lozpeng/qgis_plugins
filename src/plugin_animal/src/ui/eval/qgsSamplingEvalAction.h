#pragma once
#include <qgisinterface.h>

#include "base/QgsBaseActionFilter.h"

//!��������
typedef struct qgsEvalParams
{



}evalParams;
//!���������������������ǿ��
class qgsSamplingEvalAction :public QgsBaseActionFilter
{
	Q_OBJECT

public:
	qgsSamplingEvalAction(QgisInterface* qgsInterface);
	virtual ~qgsSamplingEvalAction();
	//!����
	int compute();
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();
private:
	//qgsSamplingParamDialog* mParamDialog; //�����Ի���
	QgisInterface* mQgsInterface;
	qgsEvalParams mParams;

private slots:
	//!
	void on_currentlayer_changed(QgsMapLayer* lyr);


};

