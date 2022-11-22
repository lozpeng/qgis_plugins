#ifndef _EXPLODE_MULTI_PARTS_ACTION_H_
#define _EXPLODE_MULTI_PARTS_ACTION_H_

#include "base/QgsBaseActionFilter.h"

#include <QObject>
#include <QVector>

class QAction;
class QToolBar;
class QgsMapLayer;

class QgisInterface;
class QgsGeometry;
/**
* ���ಿ����ɢΪС���֣�������ÿ��С���ֵ���������
*/
class qgsExplodeMultiPartsAction :public QgsBaseActionFilter
{
	Q_OBJECT

public:
	qgsExplodeMultiPartsAction();
	virtual ~qgsExplodeMultiPartsAction();
	//!����
	int compute();
protected:
	//����multipart��ɢת��Ϊ��������ͼ��
	int toSingleParts(QgsGeometry qgsGeometry, QVector<QgsGeometry>& parts);
};
#endif
