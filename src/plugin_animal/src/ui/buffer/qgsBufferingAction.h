#pragma once
#include "base/QgsBaseActionFilter.h"

#include "qgsBufferingDialog.h"

class QgsMapLayer;

class qgsBufferingAction :public QgsBaseActionFilter
{
	Q_OBJECT
public:
	qgsBufferingAction(QgisInterface* qgsInterface);
	virtual ~qgsBufferingAction();
	//!º∆À„
	int compute();
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();
private:
	qgsBufferParams mParams;
	QgisInterface* mQgsInterface;
	qgsBufferingDialog* mParamDialog;
private slots:
	//!
	void on_currentlayer_changed(QgsMapLayer* lyr);
};

