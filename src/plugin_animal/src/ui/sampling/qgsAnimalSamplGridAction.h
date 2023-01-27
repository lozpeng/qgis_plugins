#pragma once
#include <qgisinterface.h>

#include "base/QgsBaseActionFilter.h"

#include "qgsSamplingParamDialog.h"
/**
	��������������Ԫ������
*/
class qgsAnimalSamplGridAction :public QgsBaseActionFilter
{
	Q_OBJECT

public:
	qgsAnimalSamplGridAction(QgisInterface* qgsInterface);
	virtual ~qgsAnimalSamplGridAction();
	//!����
	int compute();
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();
private:
	qgsSamplingParamDialog* mParamDialog; //�����Ի���
	QgisInterface* mQgsInterface;
	qgsSampleParams mParams;

private slots:
	//!
	void on_currentlayer_changed(QgsMapLayer* lyr);
};