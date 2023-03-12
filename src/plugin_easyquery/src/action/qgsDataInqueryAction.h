#pragma once
#include "base/QgsBaseActionFilter.h"

#include "ui/qgslayerfieldsbasedialog.h"

#include <qgsvectorlayer.h>

class QgsVectorLayer;
//!Õº≤„≤È—Ø
class qgsDataInqueryAction :public QgsBaseActionFilter
{
public:
	qgsDataInqueryAction();
	~qgsDataInqueryAction();
	virtual int compute();
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();

private:
	QDialog* mParamDialog;
};

