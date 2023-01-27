#pragma once
#include <QObject>
#include <qgsrectangle.h>
#include <qgis.h>
#include <qgsvectorlayer.h>
#include <qgsrectangle.h>
#include <qgsproviderregistry.h>

//!地理处理工具类
class qgsGeoUtils : public QObject
{
Q_OBJECT
public:
	//! 3°带EPSG代码
	static int get3DegEpsgZoneId(int medX);
	//！根据中央经线获取不带带号的EPSG 编号
	static int get3DegEPSGId(int medX);
	//! 6°带EPSG代码
	static int get3DegEpsgCMId(int medX);

	static int get3DegEpsgID(double lon);
	//! 3°带带号
	static int get3DegreeNum(double lon);
	//! 6°带带号
	static int get6DegreeNum(double lon);

	//! 6°带获取EPSG编号
	static int get6DegEPSGId(double lon);

	static int getEPSGCode(QgsRectangle qgsRect);

	static bool createShpFile(const QString& filePath, 
								QgsWkbTypes::Type geoType, 
								QList<QPair<QString, QString>> fields, int epsgid);

	static QgsVectorLayer* createMemLayer(QgsWkbTypes::Type geoType, QString epsgid, QString name,QString fields);
	//!打开SHP矢量数据图层
	static QgsVectorLayer* openShpLayer(QString filePath);

	static QString toTypeStr(QgsWkbTypes::Type geoType);
};