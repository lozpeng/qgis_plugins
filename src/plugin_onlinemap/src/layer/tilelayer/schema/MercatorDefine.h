#pragma once

#include "../qtileinfo.h"
#include <qmath.h>
#include <qgspoint.h>

namespace geotile {
	#define M_MAX_VALUE	20037508.3427892
	#define M_MIN_VALUE	-20037508.3427892
	#define WIDTH_VALUE 40075016.6855784

	//�����¼���ȼ� ʹ�����µ��㷨 pos���봫��γ������
	inline long GetMercatorIndex_X(double pos, int level)
	{
		//��Ϊ�����ݵĵ�0���ǰ�����[M_MIN_VALUE,M_MAX_VALUE]�ֵģ�
		//���Ծ��ȣ�x���򣩱���Ϊ4���� -2,-1,0,1
		//γ�ȣ�y���򣩱���Ϊ2���� -1, 0
		//ÿ���·�һ������ô�Ƕ���С2

		return long((pos - M_MIN_VALUE) * (1 << level) / WIDTH_VALUE);

	}

	inline long GetMercatorIndex_Y(double pos, int level)
	{
		//��Ϊ�����ݵĵ�0���ǰ�����[M_MIN_VALUE,M_MAX_VALUE]�ֵģ�
		//���Ծ��ȣ�x���򣩱���Ϊ4���� -2,-1,0,1
		//γ�ȣ�y���򣩱���Ϊ2���� -1, 0
		//ÿ���·�һ������ô�Ƕ���С2

		return long((M_MAX_VALUE - pos) * (1 << level) / WIDTH_VALUE);

	}

	//���ݼ����������ֵ
	inline double GetMercatorCoord_X(long indx, int level)
	{
		double mapSize = (double)(WIDTH_VALUE / (1 << level));
		return M_MIN_VALUE + indx * mapSize;
	}

	inline double GetMercatorCoord_Y(long indx, int level)
	{
		double mapSize = (double)(WIDTH_VALUE / (1 << level));
		return M_MAX_VALUE - indx * mapSize - mapSize;
	}

	//����ĳһ������ĽǶȿ���
	inline double GetMercatorLevelWidth(int level)
	{
		return WIDTH_VALUE / (1 << level);
	}

	//�������кż����������
	inline void GetMercatorBlockRect(long x, long y, int level, ENVELOPE& extent)
	{
		double width = (double)(WIDTH_VALUE / (1 << level));
		extent.minx = M_MIN_VALUE + x * width;
		extent.maxx = extent.minx + width;

		extent.maxy = M_MAX_VALUE - y * width;
		extent.miny = extent.maxy - width;
	}

	//�������кż�����Ƭ��Χ
	inline void GetMercatorBlockRect(long x, long y, int level, QgsRectangle& rect)
	{
		double width = (double)(WIDTH_VALUE / (1 << level));
		rect.setXMinimum(M_MIN_VALUE + x * width);
		rect.setXMaximum(rect.xMinimum() + width);

		rect.setYMaximum(M_MAX_VALUE - y * width);
		rect.setYMinimum(rect.yMaximum() - width);
	}

	inline bool	GetMercatorBlockRect(LONGLONG blockID, ENVELOPE& bound)
	{
		int lx, ly, level;
		DecodeBlockID_L(lx, ly, level, blockID);
		GetMercatorBlockRect(lx, ly, level, bound);
		return true;
	}


	//ī��������ת��γ��
	//��γ��תī����
	inline QgsPointXY lonLat2Mercator(QgsPointXY lonLat)
	{
		QgsPointXY mercator;
		double x = lonLat.x() * M_MAX_VALUE / 180;

		double y = log(tan((90 + lonLat.y()) * M_PI / 360)) / (M_PI / 180);
		y = y * M_MAX_VALUE / 180;
		mercator.setX(x);
		mercator.setY(y);
		return mercator;
	}
	//ī����ת��γ��
	inline QgsPointXY Mercator2lonLat(QgsPointXY mercator)
	{
		QgsPointXY lonLat;
		double x = mercator.x() / M_MAX_VALUE * 180;
		double y = mercator.y() / M_MAX_VALUE * 180;
		y = 180 / M_PI * (2 * atan(exp(y * M_PI / 180)) - M_PI / 2);
		lonLat.setX(x);
		lonLat.setY(y);
		return lonLat;
	}
}