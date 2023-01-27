#include "qgsGridUtil.h"

#include "qgsGeoUtils.h"
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgis.h>
#include <qgslinestring.h>
#include <qgsfeature.h>
#include <qgspolygon.h>
#include <qgsvectorlayereditbuffer.h>
#include <qgsgeometry.h>

//! ���ɸ���
int qgsGridUtil::genGrids(QgsGeometry bndGeometry, int srcEpsgId, int distance, QgsVectorLayer* lyr, GridType gridType)
{
	QgsRectangle qgsRect = bndGeometry.boundingBox();
	double maxY = qgsRect.yMaximum();
	QgsGeometry qgsGeom;
	qgsGeom.fromWkb(bndGeometry.asWkb());
	int destEpsgCode = -1;
	if (maxY > -90.0 && maxY < 90.0) //����Ǿ�γ������,������ݽ���
	{
		destEpsgCode = qgsGeoUtils::getEPSGCode(qgsRect); //���ݷ�Χȷ�����ȴ������ȴ�ͶӰ��EPSG���
		QgsCoordinateReferenceSystem destCrs(destEpsgCode, QgsCoordinateReferenceSystem::EpsgCrsId);
		QgsCoordinateTransform coordTrans;
		coordTrans.setDestinationCrs(destCrs);
		QgsCoordinateReferenceSystem srcCrs(srcEpsgId, QgsCoordinateReferenceSystem::EpsgCrsId);
		coordTrans.setSourceCrs(srcCrs);

		Qgis::GeometryOperationResult result = qgsGeom.transform(coordTrans);
		if (result != Qgis::GeometryOperationResult::Success)
			return -1;
	}
	else destEpsgCode = srcEpsgId;

	QString fields = "field=minx:double&field=maxx:double&field=miny:double&field=maxy:double";
	if (!lyr) //���û��ָ�����ݴ洢ͼ�㣬�����ڴ����ݴ洢ͼ��
		lyr = qgsGeoUtils::createMemLayer(QgsWkbTypes::Polygon, "EPSG:" + srcEpsgId, QString::fromLocal8Bit("��������"), fields);

	if (!lyr)return -1;

	QgsVectorLayer* gridPointLayer = qgsGeoUtils::createMemLayer(QgsWkbTypes::Point, QString("EPSG:%1").arg(srcEpsgId), 
																		QString::fromLocal8Bit("�����������ĵ�"), fields);
	lyr->startEditing();
	gridPointLayer->startEditing();

	QgsCoordinateTransform coordTrans;
	coordTrans.setDestinationCrs(QgsCoordinateReferenceSystem(srcEpsgId, QgsCoordinateReferenceSystem::EpsgCrsId));
	coordTrans.setSourceCrs(QgsCoordinateReferenceSystem(destEpsgCode, QgsCoordinateReferenceSystem::EpsgCrsId));
	
	QgsFeatureList gridFeatures;
	QgsFeatureList centriods;
	int grids=0;
	if (gridType == GridType::RECTANGLE)
		grids = genRectangles(qgsGeom,distance,lyr->fields().count(), coordTrans, gridFeatures, centriods);
	else if(gridType == GridType::HEXAGON)
		grids = genHexagons(qgsGeom, distance, lyr->fields().count(), coordTrans, gridFeatures, centriods);
	else if (gridType == GridType::HEXAGON2)
		grids = genHexagons2(qgsGeom, distance, lyr->fields().count(), coordTrans, gridFeatures, centriods);

	if (gridFeatures.count() >= 1)
	{
		lyr->editBuffer()->addFeatures(gridFeatures);
		lyr->commitChanges();
		lyr->endEditCommand();
		lyr->updateExtents();
	}
	if (centriods.count()>=1)
	{
		gridPointLayer->editBuffer()->addFeatures(centriods);
		gridPointLayer->commitChanges();
		gridPointLayer->endEditCommand();
		QgsProject::instance()->addMapLayer(gridPointLayer);
		gridPointLayer->updateExtents();
	}
	return grids;
}

//!���ɾ��ο�
int qgsGridUtil::genRectangles(QgsGeometry bndPrjGeom,int distance,int fieldCount, QgsCoordinateTransform coordTrans,
		QgsFeatureList& gridFeatures, QgsFeatureList& gridCentreIds)
{
	QgsRectangle bndPrjRect = bndPrjGeom.boundingBox(); //�Ѿ�������ͶӰ	
	double dist = distance * 1000;

	double minx = bndPrjRect.xMinimum();
	int db = qRound(minx / dist) - 1;
	minx = db * dist;

	db = qRound(bndPrjRect.xMaximum() / dist) + 1;
	double maxx = db * dist;

	db = qRound(bndPrjRect.yMinimum() / dist) - 1;
	double miny = db * dist;

	db = qRound(bndPrjRect.yMaximum() / dist) + 1;
	double maxy = db * dist;

	int grids = 0;
	double lastX = minx;
	double lastY = miny;
	for (double x = minx; x < maxx; )
	{
		x += dist;
		for (double y = miny; y < maxy;)
		{
			y += dist;
			QgsGeometry geom;
			//ͶӰ����
			QgsPolylineXY polyLine;
			polyLine.append(QgsPoint(lastX, lastY));
			polyLine.append(QgsPoint(lastX, y));
			polyLine.append(QgsPoint(x, y));
			polyLine.append(QgsPoint(x, lastY));
			polyLine.append(QgsPoint(lastX, lastY));

			QgsPolygonXY polygon;
			polygon.append(polyLine);
			geom = QgsGeometry::fromPolygonXY(polygon);

			QgsFeature f;
			if (!geom.isEmpty() && bndPrjGeom.intersects(geom)) //�����ѡ��ļ���ͼ���ཻ�򱣴�
			{
				geom.transform(coordTrans); //ת��Ϊ��γ������
				f.setGeometry(geom);
				f.initAttributes(fieldCount);
				f.setAttributes(QgsAttributes() << QVariant(grids) << QVariant(lastX)
					<< QVariant(lastY) << QVariant(x) << QVariant(y));
				gridFeatures << f;

				//�������ĵ�
				{
					QgsGeometry smpPoint = geom.centroid();
					QgsFeature fp;
					fp.setGeometry(smpPoint);
					fp.initAttributes(fieldCount);
					gridCentreIds << fp;
				}
				grids++;
			}
			lastY = y;
		}
		lastX = x;
		lastY = miny;
	}
	return grids;
}

//!���ɷ���ͼ Flat Orientation
int qgsGridUtil::genHexagons(QgsGeometry bndPrjGeom, int distance, int fieldCount, QgsCoordinateTransform coordTrans,
	QgsFeatureList& gridFeatures, QgsFeatureList& gridCentreIds)
{
	QgsRectangle bndPrjRect = bndPrjGeom.boundingBox(); //�Ѿ�������ͶӰ
	int dist = distance * 1000;

	double minx = bndPrjRect.xMinimum();
	int db = qRound(minx / dist) - 1;
	minx = db * dist;

	db = qRound(bndPrjRect.xMaximum() / dist) + 1;
	double maxx = db * dist;

	db = qRound(bndPrjRect.yMinimum() / dist) - 1;
	double miny = db * dist;

	db = qRound(bndPrjRect.yMaximum() / dist) + 1;
	double maxy = db * dist;

	double y = miny - dist;
	
	//FLAT ORIENTATION:
	//�Թ������񽻵�Ϊ���ģ��������������꣬�����������ɣ��߳�Ϊ�û����ò������ȵ�һ��
	double s = dist / 2.0;
	double a = std::sqrt(std::pow(s, 2) - (std::pow(s / 2, 2))); //sqrt(3)*(s/2)

	int xCols = (maxx - minx) / (2 * a)  + 1;
	int yRows = (maxy - miny) / (2 * s)  + 1;
	int grids = 0;
	for (int i = 0; i <= yRows ; i++)
	{
		for (int j = 0; j <= xCols; j++)
		{
			double centX = minx + j * ((3 * s) / 2);
			double centY = miny + (j % 2) * a + 2 * i * a;
			QgsPolylineXY polyLine;
			polyLine.append(QgsPoint(centX + (s / 2), centY + a));	//point 1: (X + (s / 2), Y - a)
			polyLine.append(QgsPoint(centX + s, centY));			//point 2 : (X + s, Y)
			polyLine.append(QgsPoint(centX + (s / 2), centY - a));	//point 3 : (X + (s / 2), Y + a)
			polyLine.append(QgsPoint(centX - (s / 2), centY - a));	//point 4 : (X - (s / 2), Y + a)
			polyLine.append(QgsPoint(centX - s, centY));			//point 5 : (X - s, Y)
			polyLine.append(QgsPoint(centX - (s / 2), centY + a));	//point 6 : (X - (s / 2), Y - a)
			polyLine.append(QgsPoint(centX + (s / 2), centY + a));

			QgsPolygonXY polygon;
			polygon.append(polyLine);
			QgsGeometry geom;
			geom = QgsGeometry::fromPolygonXY(polygon);
			QgsFeature f;
			if (!geom.isEmpty())//&& qgsGeom.intersects(geom)) //�����ѡ��ļ���ͼ���ཻ�򱣴�
			{
				geom.transform(coordTrans); //ת��Ϊ��γ������
				f.setGeometry(geom);
				f.initAttributes(fieldCount);
				f.setAttributes(QgsAttributes() 
									<< QVariant(grids)
									<< QVariant(centX + (s / 2))
									<< QVariant(centY + a)
									<< QVariant(centX) 
									<< QVariant(centY));
				gridFeatures << f;
				grids++;
			}
			//�������ĵ�
			{
				QgsGeometry smpPoint = geom.centroid();
				QgsFeature fp;
				fp.setGeometry(smpPoint);
				fp.initAttributes(fieldCount);
				gridCentreIds << fp;
			}
		}
	}
	return grids;
}
//!���ɷ���ͼ Point 
int qgsGridUtil::genHexagons2(QgsGeometry bndPrjGeom, int distance, int fieldCount, QgsCoordinateTransform coordTrans,
										QgsFeatureList& gridFeatures, QgsFeatureList& gridCentreIds)
{
	QgsRectangle bndPrjRect = bndPrjGeom.boundingBox(); //�Ѿ�������ͶӰ
	//
	int dist = distance * 1000;

	double minx = bndPrjRect.xMinimum();
	int db = qRound(minx / dist) - 1;
	minx = db * dist;

	db = qRound(bndPrjRect.xMaximum() / dist) + 1;
	double maxx = db * dist;

	db = qRound(bndPrjRect.yMinimum() / dist) - 1;
	double miny = db * dist;

	db = qRound(bndPrjRect.yMaximum() / dist) + 1;
	double maxy = db * dist;

	double y = miny - dist;

	//Pointy ORIENTATION:
	//�Թ������񽻵�Ϊ���ģ��������������꣬�����������ɣ��߳�Ϊ�û����ò������ȵ�һ��
	double s = dist / 2.0;
	double a = std::sqrt(std::pow(s, 2) - (std::pow(s / 2, 2))); //sqrt(3)*(s/2)
	int grids = 0;
	int xCols = (maxx - minx) / (2 * s) + 1;
	int yRows = (maxy - miny) / (2 * a) + 1;
	for (int i = 0; i <= yRows; i++)
	{
		for (int j = 0; j <= xCols; j++)
		{
			double centX = minx + (i % 2) * a + 2 * j * a;
			double centY = miny +  i * ( (3 * s) /2);
			QgsPolylineXY polyLine;
			polyLine.append(QgsPoint(centX, centY - s));				//point 1: (X,Y-s)
			polyLine.append(QgsPoint(centX + a, centY - (s/2)));		//point 2 : (X+a,Y-(s/2))
			polyLine.append(QgsPoint(centX + a, centY + (s/2)));		//point 3 : (X+a, Y + (s/2))
			polyLine.append(QgsPoint(centX, centY + s));				//point 4 : (X,Y+s)
			polyLine.append(QgsPoint(centX - a, centY + (s/2)));		//point 5 : (X - a, Y+(s/2))
			polyLine.append(QgsPoint(centX - a, centY - (s/2)));		//point 6 : (X-a,Y -(s/2))
			polyLine.append(QgsPoint(centX, centY - s));

			QgsPolygonXY polygon;
			polygon.append(polyLine);
			QgsGeometry geom;
			geom = QgsGeometry::fromPolygonXY(polygon);
			QgsFeature f;
			if (!geom.isEmpty())//&& qgsGeom.intersects(geom)) //�����ѡ��ļ���ͼ���ཻ�򱣴�
			{
				geom.transform(coordTrans); //ת��Ϊ��γ������
				f.setGeometry(geom);
				f.initAttributes(fieldCount);
				f.setAttributes(QgsAttributes()
					<< QVariant(grids)
					<< QVariant(centX - a)
					<< QVariant(centY - s)
					<< QVariant(centX)
					<< QVariant(centY));
				gridFeatures << f;
				grids++;
			}
			//�������ĵ�
			{
				QgsGeometry smpPoint = geom.centroid();
				QgsFeature fp;
				fp.setGeometry(smpPoint);
				fp.initAttributes(fieldCount);
				gridCentreIds << fp;
			}
		}
	}
	return grids;
}

//!��鼸��ͼ���Ƿ�����Ҫ��
bool qgsGridUtil::checkGeometry(QgsGeometry bndGeom)
{
	return false;
}
