#pragma once
#include "base/QgsBaseActionFilter.h"

#include "ui/qgslayerfieldsbasedialog.h"

#include <qgsvectorlayer.h>

class QgsVectorLayer;
//!ͼ���ѯ
class qgsLayerQueryAction:public QgsBaseActionFilter
{
public:
	qgsLayerQueryAction();
	~qgsLayerQueryAction();
	//!���õ�ǰ����ͼ��
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

