#include "qgsExplodeMultiPartsAction.h"

#include <qgsvectorlayer.h>
#include <qgsmapcanvas.h>
#include <qgisinterface.h>
#include <qgsproject.h>
#include <qgsvectorlayereditbuffer.h>


qgsExplodeMultiPartsAction::qgsExplodeMultiPartsAction()
	: QgsBaseActionFilter(
		QgsBaseActionDescription(
			"mFeatureExplodeAction",
			QString::fromLocal8Bit("要素打散"),
			QString::fromLocal8Bit("将合并在一起的多要素打散为各个子部分，并保留属性数据。"),
			":/easy_query/images/build-24px.svg"))
{

}
qgsExplodeMultiPartsAction::~qgsExplodeMultiPartsAction()
{
}
//!计算  
int qgsExplodeMultiPartsAction::compute()
{
	if (!m_qgsInterface)return -1;
	//1..获取当前选中的图层，并判断是否处于编辑状态
	//QgsVectorLayer* pCurrentLayer = m_qgsInterface->
	QgsMapCanvas* mapCanvas = m_qgsInterface->mapCanvas();
	auto layr = mapCanvas->currentLayer();
	QgsVectorLayer* vctLyr = qobject_cast<QgsVectorLayer*>(layr);
	if (!vctLyr|| !vctLyr->isEditable())return -1;
	if (vctLyr->selectedFeatureCount() <= 0)return -1;
	QgsFeatureList selectFeatures = vctLyr->selectedFeatures();
	int count = selectFeatures.count();
	for (int i = 0; i < count; i++)
	{
		QgsFeature gFeature = selectFeatures.at(i);
		if (!gFeature.hasGeometry())continue;
		QgsAttributes qgsAttrs = gFeature.attributes();//获取所有的属性数据，用于新建的要素
		QgsGeometry qgsGeometry = gFeature.geometry();
		if (qgsGeometry.isEmpty())continue;
		//将Geometry转换的多部件,并判断是否为多部件对象，如果是，则进行单体化
		if (qgsGeometry.convertToMultiType()&& qgsGeometry.isMultipart())
		{
			QVector<QgsGeometry> parts;
			int result = toSingleParts(qgsGeometry, parts);
			if (result >= 1 && parts.count() > 1) //如果返回多个几何图形
			{
				QgsFeatureList newFeatures;
				Q_FOREACH(QgsGeometry geo,parts) // 建立新的几何图形
				{
					QgsFeature nFeature;
					nFeature.initAttributes(gFeature.attributeCount());
					nFeature.setAttributes(qgsAttrs);
					nFeature.setGeometry(geo);
					newFeatures.append(nFeature);
				}
				vctLyr->editBuffer()->deleteFeature(gFeature.id()); //先删除，这种方式将保留撤销操作
				vctLyr->editBuffer()->addFeatures(newFeatures);//添加新建的要素
			}
		}
	}
	return 1;
}
/**
*  把multipart 的geometry 转换为单部分几何要素
*/
int qgsExplodeMultiPartsAction::toSingleParts(QgsGeometry qgsGeometry, QVector<QgsGeometry>& parts)
{
	QVector<QgsGeometry> geos = qgsGeometry.asGeometryCollection();
	if (geos.count() == 1)
	{
		parts.append(geos.at(0));
		return 1;
	}
	int count = 0;
	Q_FOREACH(QgsGeometry geo, geos)
	{
		if (!geo.isEmpty()) // 如果几何要素不为空，且为多部件
		{
			if(geo.isMultipart())
				toSingleParts(geo, parts);
			else
			{
				parts.append(geo);
				count++;
			}
		}
	}
	return count;
}