#pragma once
#include "base/QgsBaseActionFilter.h"

#include "ui/qgslayerfieldsbasedialog.h"

#include <qgsvectorlayer.h>

#include "ui/qgsDataInqueryDialog.h"
class QgsVectorLayer;
class qgsDataInqueryDialog;
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
	qgsDataInqueryDialog* mParamDialog;
	sDataInqueryParams mParams;
};

