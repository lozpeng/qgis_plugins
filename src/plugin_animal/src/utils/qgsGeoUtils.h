#pragma once
#include <QObject>
#include <qgsrectangle.h>
#include <qgis.h>
#include <qgsvectorlayer.h>
#include <qgsrectangle.h>
#include <qgsproviderregistry.h>

//!����������
class qgsGeoUtils : public QObject
{
Q_OBJECT
public:
	//! 3���EPSG����
	static int get3DegEpsgZoneId(int medX);
	//���������뾭�߻�ȡ�������ŵ�EPSG ���
	static int get3DegEPSGId(int medX);
	//! 6���EPSG����
	static int get3DegEpsgCMId(int medX);

	static int get3DegEpsgID(double lon);
	//! 3�������
	static int get3DegreeNum(double lon);
	//! 6�������
	static int get6DegreeNum(double lon);

	//! 6�����ȡEPSG���
	static int get6DegEPSGId(double lon);

	static int getEPSGCode(QgsRectangle qgsRect);

	static bool createShpFile(const QString& filePath, 
								QgsWkbTypes::Type geoType, 
								QList<QPair<QString, QString>> fields, int epsgid);

	static QgsVectorLayer* createMemLayer(QgsWkbTypes::Type geoType, QString epsgid, QString name,QString fields);
	//!��SHPʸ������ͼ��
	static QgsVectorLayer* openShpLayer(QString filePath);

	static QString toTypeStr(QgsWkbTypes::Type geoType);
};