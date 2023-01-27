#pragma once

#include <qgsgeometry.h>
#include <qgsvectorlayer.h>
enum class GridType {
	RECTANGLE = 1,
	HEXAGON = 2,
	HEXAGON2 = 3
};
//���������ɰ����࣬���������ݴ�����ڴ���
class qgsGridUtil
{
public:
	static int genGrids(QgsGeometry bndGeometry, int srcEpsgId, int distance, 
								QgsVectorLayer* lyr, GridType gridType);

	//!���ɾ��ο�
	static int genRectangles(QgsGeometry bndPrjGeom, int distance, int fieldCount, 
		QgsCoordinateTransform coordTrans,
		QgsFeatureList& gridFeatures, QgsFeatureList& gridCentreIds);
	//!���ɷ���ͼ Flat 
	static int genHexagons(QgsGeometry bndPrjGeom, int distance, int fieldCount,
		QgsCoordinateTransform coordTrans,
		QgsFeatureList& gridFeatures, QgsFeatureList& gridCentreIds);
	//!���ɷ���ͼ Point 
	static int genHexagons2(QgsGeometry bndPrjGeom, int distance, int fieldCount, 
		QgsCoordinateTransform coordTrans,
		QgsFeatureList& gridFeatures, QgsFeatureList& gridCentreIds);

	//!��鼸��ͼ���Ƿ�����Ҫ��
	static bool checkGeometry(QgsGeometry geom);
};

