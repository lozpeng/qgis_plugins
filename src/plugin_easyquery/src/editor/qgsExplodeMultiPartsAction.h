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
* 将多部件打散为小部分，并保留每个小部分的属性数据
*/
class qgsExplodeMultiPartsAction :public QgsBaseActionFilter
{
	Q_OBJECT

public:
	qgsExplodeMultiPartsAction();
	virtual ~qgsExplodeMultiPartsAction();
	//!计算
	int compute();
protected:
	//！将multipart打散转换为单个几何图形
	int toSingleParts(QgsGeometry qgsGeometry, QVector<QgsGeometry>& parts);
};
#endif
