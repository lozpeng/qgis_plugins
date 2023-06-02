#pragma once
#include <qgisinterface.h>

#include "base/QgsBaseActionFilter.h"

class qgsFaunaListAction :public QgsBaseActionFilter
{
	Q_OBJECT

public:
	qgsFaunaListAction(QgisInterface* qgsInterface);
	virtual ~qgsFaunaListAction();
	//!º∆À„
	int compute();
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();
private:

	QgisInterface* mQgsInterface;
private slots:
};

