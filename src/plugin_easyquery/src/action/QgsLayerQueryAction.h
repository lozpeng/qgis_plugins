#pragma once
#include "base/QgsBaseActionFilter.h"

#include "ui/qgsLayerQueryDialog.h"
#include "ui/qgsLayerFieldsBaseDialog.h"

class QgsVectorLayer;
//!图层查询
class qgsLayerQueryAction:public QgsBaseActionFilter
{
public:
	qgsLayerQueryAction();
	~qgsLayerQueryAction();
	//!设置当前图层
	void setCurrentLayer(QgsVectorLayer* pLayer);
	//!计算
	int compute() { return 0; }
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();

	QgsVectorLayer* mVectorLayer;
	qgsLayerQueryDialog* mSearchDialog;
	qgsLayerFieldsBaseDialog* mFieldsSelectorDlg;
};

