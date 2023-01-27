#pragma once
#include "base/QgsBaseActionFilter.h"

#include "ui/qgslayerfieldsbasedialog.h"

#include <qgsvectorlayer.h>

class QgsVectorLayer;
//!图层查询
class qgsLayerQueryAction:public QgsBaseActionFilter
{
public:
	qgsLayerQueryAction();
	~qgsLayerQueryAction();
	//!设置当前处理图层
	void setCurrentLayer(QgsVectorLayer* pLayer);

	virtual int compute();
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();

private :
	qgsLayerFieldsBaseDialog* mFieldsSelectorDlg;
	QgsVectorLayer* mVectorLayer;
	QDialog* mSearchDialog;
};

