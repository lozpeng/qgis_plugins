#pragma once
#include "base/QgsBaseActionFilter.h"

#include "ui/qgsLayerQueryDialog.h"
#include "ui/qgsLayerFieldsBaseDialog.h"

class QgsVectorLayer;
//!ͼ���ѯ
class qgsLayerQueryAction:public QgsBaseActionFilter
{
public:
	qgsLayerQueryAction();
	~qgsLayerQueryAction();
	//!���õ�ǰͼ��
	void setCurrentLayer(QgsVectorLayer* pLayer);
	//!����
	int compute() { return 0; }
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();

	QgsVectorLayer* mVectorLayer;
	qgsLayerQueryDialog* mSearchDialog;
	qgsLayerFieldsBaseDialog* mFieldsSelectorDlg;
};

