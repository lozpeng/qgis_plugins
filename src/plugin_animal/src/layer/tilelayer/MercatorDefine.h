#ifndef __EVTERRAINDEFINE_H_
#define __EVTERRAINDEFINE_H_

#include "tile.h"
#include <qmath.h>
///////////////////////////////////////////////////////////////
/*
Web墨卡托投影坐标系，广泛应用与Google Map、Bing Map等地图投影中。它以整个世界范围，赤道作为标准纬线，本初子午线作为中央经线，两者交点为坐标原点，向东向北为正，向西向南为负。
X轴：赤道半径取值为6378137米，则赤道周长为2*PI*r ，以坐标原点为中心，东西南北各方向为其值一半，即 PI*r= 20037508.3427892，因此X轴的取值范围：[-20037508.3427892,20037508.3427892]。
Y轴：由墨卡托投影的公式可知，当纬度φ接近两极，即90°时，y值趋向于无穷。为了使用方便，就把Y轴的取值范围也限定在[-20037508.3427892,20037508.3427892]之间，形成一个正方形。
因此在投影坐标系（米）下的范围是：最小(-20037508.3427892, -20037508.3427892 )到最大 (20037508.3427892, 20037508.3427892)。经过反算，可得到纬度85.05112877980659。因此纬度取值范围是[-85.05112877980659，85.05112877980659]。
*/
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
	return M_MIN_VALUE+indx * mapSize;
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

inline bool	GetMercatorBlockRect(LONGLONG blockID, ENVELOPE &bound)
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

#endif //__VMTERRAINDEFINE_H_