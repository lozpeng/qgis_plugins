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
			QString::fromLocal8Bit("Ҫ�ش�ɢ"),
			QString::fromLocal8Bit("���ϲ���һ��Ķ�Ҫ�ش�ɢΪ�����Ӳ��֣��������������ݡ�"),
			":/easy_query/images/build-24px.svg"))
{

}
qgsExplodeMultiPartsAction::~qgsExplodeMultiPartsAction()
{
}
//!����  
int qgsExplodeMultiPartsAction::compute()
{
	if (!m_qgsInterface)return -1;
	//1..��ȡ��ǰѡ�е�ͼ�㣬���ж��Ƿ��ڱ༭״̬
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
		QgsAttributes qgsAttrs = gFeature.attributes();//��ȡ���е��������ݣ������½���Ҫ��
		QgsGeometry qgsGeometry = gFeature.geometry();
		if (qgsGeometry.isEmpty())continue;
		//��Geometryת���Ķಿ��,���ж��Ƿ�Ϊ�ಿ����������ǣ�����е��廯
		if (qgsGeometry.convertToMultiType()&& qgsGeometry.isMultipart())
		{
			QVector<QgsGeometry> parts;
			int result = toSingleParts(qgsGeometry, parts);
			if (result >= 1 && parts.count() > 1) //������ض������ͼ��
			{
				QgsFeatureList newFeatures;
				Q_FOREACH(QgsGeometry geo,parts) // �����µļ���ͼ��
				{
					QgsFeature nFeature;
					nFeature.initAttributes(gFeature.attributeCount());
					nFeature.setAttributes(qgsAttrs);
					nFeature.setGeometry(geo);
					newFeatures.append(nFeature);
				}
				vctLyr->editBuffer()->deleteFeature(gFeature.id()); //��ɾ�������ַ�ʽ��������������
				vctLyr->editBuffer()->addFeatures(newFeatures);//����½���Ҫ��
			}
		}
	}
	return 1;
}
/**
*  ��multipart ��geometry ת��Ϊ�����ּ���Ҫ��
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
		if (!geo.isEmpty()) // �������Ҫ�ز�Ϊ�գ���Ϊ�ಿ��
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