#pragma once
#include "base/QgsBaseActionFilter.h"


class QgsVectorLayer;
//!ͼ���ѯ
class qgsLayerQueryAction:public QgsBaseActionFilter
{
public:
	qgsLayerQueryAction();
	~qgsLayerQueryAction();
protected:
	//inherited from BaseFilter
	virtual int openInputDialog();
	virtual void getParametersFromDialog();
};

