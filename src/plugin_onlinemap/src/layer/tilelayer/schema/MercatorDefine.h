#pragma once

#include "../qtileinfo.h"
#include <qmath.h>
#include <qgspoint.h>

namespace geotile {
	#define M_MAX_VALUE	20037508.3427892
	#define M_MIN_VALUE	-20037508.3427892
	#define WIDTH_VALUE 40075016.6855784

	//在球下计算等级 使用如下的算法 pos必须传经纬度坐标
	inline long GetMercatorIndex_X(double pos, int level)
	{
		//因为球数据的第0级是按照来[M_MIN_VALUE,M_MAX_VALUE]分的，
		//所以经度（x方向）被分为4部分 -2,-1,0,1
		//纬度（y方向）被分为2部分 -1, 0
		//每往下分一级，那么角度缩小2

		return long((pos - M_MIN_VALUE) * (1 << level) / WIDTH_VALUE);

	}

	inline long GetMercatorIndex_Y(double pos, int level)
	{
		//因为球数据的第0级是按照来[M_MIN_VALUE,M_MAX_VALUE]分的，
		//所以经度（x方向）被分为4部分 -2,-1,0,1
		//纬度（y方向）被分为2部分 -1, 0
		//每往下分一级，那么角度缩小2

		return long((M_MAX_VALUE - pos) * (1 << level) / WIDTH_VALUE);

	}

	//根据级别计算坐标值
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

	//返回某一个级别的角度宽度
	inline double GetMercatorLevelWidth(int level)
	{
		return WIDTH_VALUE / (1 << level);
	}

	//根据行列号计算地理坐标
	inline void GetMercatorBlockRect(long x, long y, int level, ENVELOPE& extent)
	{
		double width = (double)(WIDTH_VALUE / (1 << level));
		extent.minx = M_MIN_VALUE + x * width;
		extent.maxx = extent.minx + width;

		extent.maxy = M_MAX_VALUE - y * width;
		extent.miny = extent.maxy - width;
	}

	//根据行列号计算瓦片范围
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


	//墨卡托坐标转经纬度
	//经纬度转墨卡托
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
	//墨卡托转经纬度
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