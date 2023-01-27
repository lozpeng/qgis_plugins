// Andrew Naplavkov

#include <cmath>

#include "tile.h"

// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Tile_numbers_to_lon..2Flat.
QgsPointXY Tile::toPoint() const
{
  const double Pi = 3.1415926535897932384;
  const double n(std::pow(2., double(z)));
  const double lon_deg = double(x) / n * 360.0 - 180.0;
  const double lat_rad = std::atan(std::sinh(Pi * (1. - 2. * double(y) / n)));
  const double lat_deg = lat_rad * 180.0 / Pi;
  return QgsPointXY(lon_deg, lat_deg);
}

QgsRectangle Tile::toRect() const
{
  return QgsRectangle( this->toPoint(), Tile(x + 1, y + 1, z).toPoint() );
}

long Tile::tileId() const
{
	return BLOCK_ID(x,y,z);
}
QgsPointXY Tile::toXiPoint() const
{
	double dx = SilenTileUtils::GetCoord(x,z);
	double dy = SilenTileUtils::GetCoord(y,z);
	return QgsPointXY(dx, dy);
}

QgsRectangle Tile::toXiRect() const
{
  return QgsRectangle( this->toXiPoint(), Tile(x + 1, y + 1, z).toXiPoint() );
}
//根据分辨率计算级别
double SilenTileUtils::GetLevelByDis(double dis)
{
	return (TOP_LEVEL - log(dis)/LN2);
}
long SilenTileUtils::GetNearestLevelByDis(double dis)
{
	double level = (TOP_LEVEL - log(dis)/LN2);
	level = (long)(level+0.75);
	return level;
}
//根据级别计算当前级别的宽度
double SilenTileUtils::GetDisByLevel(int level)
{
	return pow(2.0,TOP_LEVEL-level);
}
//获取下一级别的编号
void SilenTileUtils::GetLowIds(__int64 pid,__int64 *bid)
{
	long x,y,level;
	DecodeBlockID_L(x,y,level,pid);
	x <<= 1; y <<= 1; level++;
	EncodeBlockID_L(x  , y  , level, bid[0]);
	EncodeBlockID_L(x+1, y  , level, bid[1]);
	EncodeBlockID_L(x  , y+1, level, bid[2]);
	EncodeBlockID_L(x+1, y+1, level, bid[3]);
}
//
long SilenTileUtils::GetIndex(double coord,int level,int gridSize)
{
	double mapSize = pow(2.0,gridSize) * pow(2.0,TOP_LEVEL-level);
	return (floor(coord/mapSize));
}
//根据地理经纬度坐标和级别计算层块的行列号
long SilenTileUtils::GetIndex(double coord,int level)
{
	return (floor(coord * (1<<level) / 90.0));
}
//根据层块左上角地理坐标计算层块编号
LONGLONG SilenTileUtils::GetTileId(double coordx,double coordy,int level)
{
	long x = GetIndex(coordx,level);
	long y = GetIndex(coordy,level);
	return BLOCK_ID(x,y,level);
}
//根据行列号和级别行计算地理矩形坐标范围
ENVELOPE SilenTileUtils::GetRectByIndex(long xIndex,long yIndex,int level,int gridSize)
{
	ENVELOPE env;
	double mapSize = pow(2.0, gridSize) * pow(2.0, TOP_LEVEL - level);
	env.minx = xIndex * mapSize;
	env.miny = yIndex * mapSize;
	env.maxx = env.minx + mapSize;
	env.maxy = env.miny + mapSize;
	return env;
}
////根据行列号和级别行计算经纬度地理矩形坐标范围
ENVELOPE SilenTileUtils::GetRectByIndex(long xIndex,long yIndex,int level)
{
	double mapSize = (double)(90.0 / (1 << level));
	ENVELOPE env;
	env.minx = xIndex * mapSize;
	env.miny = yIndex * mapSize;
	env.maxx = env.minx + mapSize;
	env.maxy = env.miny + mapSize;
	return env;
}

//根据编号计算层块的地理经纬度矩形框坐标
ENVELOPE SilenTileUtils::GetGeoRectById(LONGLONG tileId)
{
	long x = BLOCK_ID_X(tileId);
	long y = BLOCK_ID_Y(tileId);
	long level = BLOCK_ID_LEVEL(tileId);
	return GetRectByIndex(x,y,(int)level);
}
//计算地理范围
ENVELOPE SilenTileUtils::GetGeoRectById(LONGLONG tileId,int gridSize)
{
	long x = BLOCK_ID_X(tileId);
	long y = BLOCK_ID_Y(tileId);
	long level = BLOCK_ID_LEVEL(tileId);
	return GetRectByIndex(x,y,(int)level,gridSize);
}
//根据级别计算坐标值
double SilenTileUtils::GetCoord(long indx,int level)
{
	double mapSize = (double)(90.0 / (1 << level));
	return (double)(indx * mapSize);
}
//根据行列号计算地理坐标
double SilenTileUtils::GetCoord(long indx,int level,int gridSize)
{
	double mapsize = pow(2.0, gridSize) * pow(2.0, TOP_LEVEL - level);
	return (double)(indx * mapsize);
}
//根据行列号计算地理坐标
void SilenTileUtils::GetBlockRect(long x,long y,int level,ENVELOPE& extent)
{
	double width =  (double)(90.0 / (1 << level));
	extent.minx = x * width;
	extent.maxx = extent.minx + width;

	extent.miny = y * width;
	extent.maxy = extent.miny + width;
}
//根据行列号计算瓦片范围
void SilenTileUtils::GetBlockRect(long x,long y,int level,QgsRectangle& rect)
{
	double width =  (double)(90.0 / (1 << level));
	rect.setXMinimum(x * width);
	rect.setXMaximum(rect.xMinimum() + width);

	rect.setYMinimum( y * width);
	rect.setYMaximum(rect.yMinimum()+width);
}
//计算一行的宽度
double SilenTileUtils::GetLevelWidth(int level)
{
	return (double)(90.0 / (1 << level));
}
