#pragma once
#include <QCombobox>

#include <qgisinterface.h>
#include <qgsproject.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>


class qgsLayerUtils
{

public:
	/// <summary>
	/// 获取矢量图层根据编号
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	static QgsVectorLayer* getVectorLayerById(QString id);
	/// <summary>
	/// 根据图层名称获取矢量图层
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	static QgsVectorLayer* getVectorLayerByName(QString id);
	/// <summary>
	/// 获取所有的矢量图层名称至Combobo中
	/// </summary>
	/// <param name="combox"></param>
	/// <param name="exLyrId"></param>
	static void loadVectorLayerNames(QComboBox* combox, QString exLyrId);
	/// <summary>
	/// 获取图层的字段名称
	/// </summary>
	/// <param name="combo"></param>
	/// <param name="lyrIdx"></param>
	static void loadLayerFieldsNames(QComboBox* combo,QString lyrIdx);
};

