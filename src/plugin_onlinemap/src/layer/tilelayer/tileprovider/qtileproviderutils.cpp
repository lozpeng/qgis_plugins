#include "qtileproviderutils.h"

#include "../qtileutils.h"
#include "../schema/MercatorDefine.h"
namespace geotile {

	//////////////////////////////////////////////////////
	//qTileDbUtils
	qTileProviderUtils::qTileProviderUtils()
	{

	}

	qTileProviderUtils::~qTileProviderUtils()
	{

	}
	//////////////////////////////////////////////////////////////////
	//qEdomTileDbUtils
	qEdomTileProviderUtils::qEdomTileProviderUtils() :qTileProviderUtils()
	{

	}

	qEdomTileProviderUtils::~qEdomTileProviderUtils()
	{

	}

	long qEdomTileProviderUtils::GetIndex_X(double pos, int level)
	{
		return qTileUtils::GetIndex(pos, level);
	}

	long qEdomTileProviderUtils::GetIndex_Y(double pos, int level)
	{
		return qTileUtils::GetIndex(pos, level);
	}

	//根据级别计算坐标值
	double qEdomTileProviderUtils::GetCoord_X(long index, int level)
	{
		return qTileUtils::GetCoord(index, level);
	}

	//根据级别计算坐标值
	double qEdomTileProviderUtils::GetCoord_Y(long index, int level)
	{
		return qTileUtils::GetCoord(index, level);
	}
	//根据行列号计算地理坐标
	void qEdomTileProviderUtils::GetBlockRect(long x, long y, int level, ENVELOPE& extent)
	{
		qTileUtils::GetBlockRect(x, y, level, extent);
	}

	//根据行列号计算瓦片范围
	void qEdomTileProviderUtils::GetBlockRect(long x, long y, int level, QgsRectangle& rect)
	{
		qTileUtils::GetBlockRect(x, y, level, rect);
	}

	//计算一行的宽度
	double qEdomTileProviderUtils::GetLevelWidth(int level)
	{
		return qTileUtils::GetLevelWidth(level);
	}

	//////////////////////////////////////////////////////////////////
	//qWebMecatorProviderUtils
	qWebMecatorProviderUtils::qWebMecatorProviderUtils() :qTileProviderUtils()
	{

	}

	qWebMecatorProviderUtils::~qWebMecatorProviderUtils()
	{

	}

	long qWebMecatorProviderUtils::GetIndex_X(double pos, int level)
	{
		return GetMercatorIndex_X(pos, level);
	}

	long qWebMecatorProviderUtils::GetIndex_Y(double pos, int level)
	{
		return GetMercatorIndex_Y(pos, level);
	}

	//根据级别计算坐标值
	double qWebMecatorProviderUtils::GetCoord_X(long index, int level)
	{
		return GetMercatorCoord_X(index, level);
	}

	double qWebMecatorProviderUtils::GetCoord_Y(long index, int level)
	{
		return GetMercatorCoord_Y(index, level);
	}

	//根据行列号计算地理坐标
	void qWebMecatorProviderUtils::GetBlockRect(long x, long y, int level, ENVELOPE& extent)
	{
		GetMercatorBlockRect(x, y, level, extent);
	}

	//根据行列号计算瓦片范围
	void qWebMecatorProviderUtils::GetBlockRect(long x, long y, int level, QgsRectangle& rect)
	{
		GetMercatorBlockRect(x, y, level, rect);
	}

	//计算一行的宽度
	double qWebMecatorProviderUtils::GetLevelWidth(int level)
	{
		return GetMercatorLevelWidth(level);
	}

	double qWebMecatorProviderUtils::GetRefValue()
	{
		return M_MAX_VALUE;
	}
}
