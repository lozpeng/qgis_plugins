#pragma once

#include <qgsgeometry.h>
#include <qgsvectorlayer.h>
enum class GridType {
	RECTANGLE = 1,
	HEXAGON = 2,
	HEXAGON2 = 3
};
//！网格生成帮助类，将生成数据存放在内存中
class qgsGridUtil
{
public:
	static int genGrids(QgsGeometry bndGeometry, int srcEpsgId, int distance, 
								QgsVectorLayer* lyr, GridType gridType);

	//!生成矩形框
	static int genRectangles(QgsGeometry bndPrjGeom, int distance, int fieldCount, 
		QgsCoordinateTransform coordTrans,
		QgsFeatureList& gridFeatures, QgsFeatureList& gridCentreIds);
	//!生成蜂窝图 Flat 
	static int genHexagons(QgsGeometry bndPrjGeom, int distance, int fieldCount,
		QgsCoordinateTransform coordTrans,
		QgsFeatureList& gridFeatures, QgsFeatureList& gridCentreIds);
	//!生成蜂窝图 Point 
	static int genHexagons2(QgsGeometry bndPrjGeom, int distance, int fieldCount, 
		QgsCoordinateTransform coordTrans,
		QgsFeatureList& gridFeatures, QgsFeatureList& gridCentreIds);

	//!检查几何图形是否满足要求
	static bool checkGeometry(QgsGeometry geom);
};

