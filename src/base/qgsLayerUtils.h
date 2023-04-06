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
	/// ��ȡʸ��ͼ����ݱ��
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	static QgsVectorLayer* getVectorLayerById(QString id);
	/// <summary>
	/// ����ͼ�����ƻ�ȡʸ��ͼ��
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	static QgsVectorLayer* getVectorLayerByName(QString id);
	/// <summary>
	/// ��ȡ���е�ʸ��ͼ��������Combobo��
	/// </summary>
	/// <param name="combox"></param>
	/// <param name="exLyrId"></param>
	static void loadVectorLayerNames(QComboBox* combox, QString exLyrId);
	/// <summary>
	/// ��ȡͼ����ֶ�����
	/// </summary>
	/// <param name="combo"></param>
	/// <param name="lyrIdx"></param>
	static void loadLayerFieldsNames(QComboBox* combo,QString lyrIdx);
};

