#pragma once

#include <qgspoint.h>
#include <qgsrectangle.h>

#include "qtileinfo.h"
namespace geotile
{
	//数据瓦片化帮助类
	class qTileUtils
	{
	public:
		//根据分辨率获取当前最靠近的级别
		static double GetLevelByDis(double dis);
		//获取最近的一个级别根据分辨率
		static long GetNearestLevelByDis(double dis);
		//根据级别计算当前级别的宽度
		static double GetDisByLevel(int level);
		//获取下一级别的编号
		static void GetLowIds(__int64 pid, __int64* bid);
		//
		static long GetIndex(double coord, int level, int gridSize);
		//根据地理经纬度坐标和级别计算层块的行列号
		static long GetIndex(double coord, int level);
		//根据层块左上角地理坐标计算层块编号
		static LONGLONG GetTileId(double coordx, double coordy, int level);
		//根据行列号和级别行计算地理矩形坐标范围
		static ENVELOPE GetRectByIndex(long xIndex, long yIndex, int level, int gridSize);
		////根据行列号和级别行计算经纬度地理矩形坐标范围
		static ENVELOPE GetRectByIndex(long xIndex, long yIndex, int level);
		//根据编号计算层块的地理经纬度矩形框坐标
		static ENVELOPE GetGeoRectById(LONGLONG tileId);
		//计算地理范围
		static ENVELOPE GetGeoRectById(LONGLONG tileId, int gridSize);
		//根据级别计算坐标值
		static double GetCoord(long indx, int level);
		//根据行列号计算地理坐标
		static double GetCoord(long indx, int level, int gridSize);
		//根据行列号计算地理坐标
		static void GetBlockRect(long x, long y, int level, ENVELOPE& extent);
		//根据行列号计算瓦片范围
		static void GetBlockRect(long x, long y, int level, QgsRectangle& rect);
		//计算一行的宽度
		static double GetLevelWidth(int level);
	};
}
