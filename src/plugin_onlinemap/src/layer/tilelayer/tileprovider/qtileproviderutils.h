#pragma once

#include <QObject>

#include <qgscoordinatetransform.h>
#include <qgsmapcanvas.h>
#include <qgsrectangle.h>
#include <QgsGeometry.h>

#include "../qtileutils.h"

namespace geotile {
	typedef struct qCutTileStru {
		QgsRectangle outRect;
		int minx;
		int maxx;
		int miny;
		int maxy;

		int level;
		int minIndX;
		int minIndY;
		int maxIndX;
		int maxIndY;
	}_qCutTileStru;

	class qTileProviderUtils : public QObject
	{
		Q_OBJECT

	public:
		qTileProviderUtils();
		~qTileProviderUtils();

		virtual int GetType() = 0;
		virtual long GetIndex_X(double pos, int level) = 0;
		virtual long GetIndex_Y(double pos, int level) = 0;
		//根据级别计算坐标值
		virtual double GetCoord_X(long indx, int level) = 0;
		virtual double GetCoord_Y(long indx, int level) = 0;
		//根据行列号计算地理坐标
		virtual void GetBlockRect(long x, long y, int level, ENVELOPE& extent) = 0;
		//根据行列号计算瓦片范围
		virtual void GetBlockRect(long x, long y, int level, QgsRectangle& rect) = 0;
		//计算一行的宽度
		virtual double GetLevelWidth(int level) = 0;
		virtual double GetRefValue() = 0;
	};

	//经纬度
	class qEdomTileProviderUtils : public qTileProviderUtils
	{
	public:
		qEdomTileProviderUtils();
		virtual  ~qEdomTileProviderUtils();

		int GetType() { return 1; };
		long GetIndex_X(double pos, int level);
		long GetIndex_Y(double pos, int level);
		//根据级别计算坐标值
		double GetCoord_X(long indx, int level);
		double GetCoord_Y(long indx, int level);
		//根据行列号计算地理坐标
		void GetBlockRect(long x, long y, int level, ENVELOPE& extent);
		//根据行列号计算瓦片范围
		void GetBlockRect(long x, long y, int level, QgsRectangle& rect);
		//计算一行的宽度
		double GetLevelWidth(int level);
		double GetRefValue() { return 90.0; };
	};

	//墨卡托投影
	class qWebMecatorProviderUtils : public qTileProviderUtils
	{
	public:
		qWebMecatorProviderUtils();
		virtual  ~qWebMecatorProviderUtils();

		int GetType() { return 1; };
		long GetIndex_X(double pos, int level);
		long GetIndex_Y(double pos, int level);
		//根据级别计算坐标值
		double GetCoord_X(long indx, int level);
		double GetCoord_Y(long indx, int level);
		//根据行列号计算地理坐标
		void GetBlockRect(long x, long y, int level, ENVELOPE& extent);
		//根据行列号计算瓦片范围
		void GetBlockRect(long x, long y, int level, QgsRectangle& rect);
		//计算一行的宽度
		double GetLevelWidth(int level);
		double GetRefValue();
	};
}
