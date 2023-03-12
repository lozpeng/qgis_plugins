#include "qgsGeoUtils.h"


#include <QLibrary>
#include <QFile>
#include <QDir>

#include <qgis.h>
#include <qgsvectorlayer.h>
#include <qgsrectangle.h>
#include <qgsproviderregistry.h>

//!根据中央经线坐标计算EPSG 带编号（3°带）
int qgsGeoUtils::get3DegEpsgZoneId(int medX)
{
	//EPSG 4513 对应中央经线度数为 75度
	//在中国范围内按此计算
	return 4513 + (medX - 75) / 3;
}

//！根据中央经线获取不带带号的EPSG 编号
int qgsGeoUtils::get3DegEPSGId(int medX)
{
	return 4534 + (medX - 75) / 3;
}
//!根据中央经线坐标计算EPSG Cm 编号(6°带)
int qgsGeoUtils::get3DegEpsgCMId(int medX)
{
	return 4541 + (medX - 75) / 3;
}

int qgsGeoUtils::get6DegEPSGId(double lon)
{
	if (lon < 70 || lon>138)return -1;
	//CGCS2000 / Gauss-Kruger zone 13 -- 23
	int N = qRound((lon + 3) / 6);
	double meadX = 6 * N + 3;
	return 4491 + qRound((meadX - 75) / 6.0);
}

int qgsGeoUtils::get3DegEpsgID(double lon)
{
	//3度带，中央经线计算方法
	int daihao = qRound(lon / 3);
	int midLineX = 3.0 * daihao;
	return get3DegEpsgZoneId(midLineX);
}

//!
int qgsGeoUtils::get3DegreeNum(double lon)
{
	return (lon - 1.5) / 3 + 1;
}
//!
int qgsGeoUtils::get6DegreeNum(double lon)
{
	return (lon - 3) / 6 + 1;
}
//！根据范围获取投影EPSG编号
int qgsGeoUtils::getEPSGCode(QgsRectangle qgsRect)
{
	double minX = qgsRect.xMinimum();
	double maxX = qgsRect.xMaximum();
	int id = get3DegEpsgID(minX);
	int id2 = get3DegEpsgID(maxX);
	if (id == id2)return id;
	else return get6DegEPSGId(minX);
}
//!打开
QgsVectorLayer* qgsGeoUtils::openShpLayer(QString filePath)
{
	if (!QFile::exists(filePath))  //如果文件不存在，则建立一个，默认为多边形
	{
		QList<QPair<QString, QString>> fields;
		QPair<QString, QString> field;
		field.first = QString("ID");
		field.second = QString("Integer");
		fields << field;
		bool result = createShpFile(filePath, Qgis::WkbType::Polygon, fields,4480);
		if (!result)return nullptr;
	}
	QString name = QFile(filePath).fileName();
	QgsVectorLayer* vectorLayer = new QgsVectorLayer(filePath, name);
	if (vectorLayer)
	{
		vectorLayer->isValid();
		return vectorLayer;
	}
	else return nullptr;
}
//!创建一个零时内存图层
QgsVectorLayer* qgsGeoUtils::createMemLayer(Qgis::WkbType geoType, QString epsgid, QString name, QString fields)
{
	if (epsgid <= 0)epsgid = 4480;
	if (name.isEmpty())name = "MEMORY_LYR" + qrand();
	QString typeStr = toTypeStr(geoType);
	if (!epsgid.startsWith("EPSG:"))
		epsgid = "EPSG:" + epsgid;
	if (!fields.startsWith("&"))
		fields = "&" + fields;

	QString uri = QString("%1?crs=%2&field=id:integer%3&index=yes").arg(typeStr).arg(epsgid).arg(fields);
	QgsVectorLayer* tmpLayer = new QgsVectorLayer(uri, name, "memory");
	return tmpLayer;
}
//!将空间数据类型转换为字符串
QString qgsGeoUtils::toTypeStr(Qgis::WkbType geoType)
{
	return QgsWkbTypes::displayString(geoType);
}
//!创建一个shp文件
bool qgsGeoUtils::createShpFile(const QString& filePath,
	Qgis::WkbType geoType,
										QList<QPair<QString, QString>> fields, int epsgid)
{
	if (epsgid <= 0)epsgid = 4480; //China Geodetic Coordinate System 2000
	if (fields.isEmpty()) //如果为空则加一个默认字段
	{
		QPair<QString,QString> fld;
		fld.first = "ID";
		fld.second = "Integer";
		fields << fld;
	}


	QgsProviderRegistry* pReg = QgsProviderRegistry::instance();
	QString ogrlib = pReg->library("ogr");
	// load the data provider
	QLibrary* myLib = new QLibrary(ogrlib);//使用gdal的ogr模块来创建shp文件
	bool loaded = myLib->load();
	if (loaded)
	{
		typedef bool (*createEmptyDataSourceProc)(const QString&, const QString&, 
			const QString&, Qgis::WkbType,
			const QList< QPair<QString, QString> >&, const QgsCoordinateReferenceSystem*);
		createEmptyDataSourceProc createEmptyDataSource =
				(createEmptyDataSourceProc)cast_to_fptr(myLib->resolve("createEmptyDataSource"));
		if (createEmptyDataSource)
		{
			if (geoType != Qgis::WkbType::Unknown)
			{
				QgsCoordinateReferenceSystem srs(epsgid, QgsCoordinateReferenceSystem::EpsgCrsId);
				//
				if (!createEmptyDataSource(filePath, "ESRI Shapefile", "System", geoType, fields, &srs))
					return false;
			}
			else
				return false;
		}
		else
			return false;
	}
	return true;
}

