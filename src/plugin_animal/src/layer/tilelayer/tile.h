#ifndef TILE_H
#define TILE_H
#include <limits>  
 

#include <qgspoint.h>
#include <qgsrectangle.h>

#define	OUTIMAGE_WIDTH 35 //导出影像的宽度
#define	OUTIMAGE_HEIGHT	35 //导出影像的每幅高度
#define	IMAGE_SIZE	256 //一副ID影像数据的宽度

static const double LN2 = 0.693147180;
////最高级别
static const int TOP_LEVEL = 16;
//聚合瓦片大小
static const int TILE_MATRIX = 2048;
////最高级别尺寸
//const long TOP_LEVEL_SIZE =1 << TOP_LEVEL;
////层块大小
//const long TILE_GRID = 256;
////层块尺寸
//const long TILE_SIZE = TILE_GRID * TILE_GRID;
////
//const int TILE_GEO_SIZE = 8;
//#define long MASK_28 = (1 << 28) - 1;
///
#ifndef LONGLONG
	typedef __int64 LONGLONG;
#endif
#define	__28_MASK	((1<<28)-1)

#define	GET_SIGN_28(x)		((x)|(0-((x)&(1<<27))))
	//根据层块编号计算层块的行号
#define	BLOCK_ID_X(id)		GET_SIGN_28((int)((id) & __28_MASK))
	//根据层块编号计算层块列号
#define	BLOCK_ID_Y(id)		GET_SIGN_28((int)(((id)>>28) & __28_MASK))

	//根据层块编号计算层块的级别
#define	BLOCK_ID_LEVEL(id)	(int)(((id)>>56)&0xFF)
	//根据行列号和级别计算层块的编号
#define	BLOCK_ID(x,y,level)	(((x)&__28_MASK)|((LONGLONG)((y)&__28_MASK)<<28)|((LONGLONG)(level)<<56))

	//将行列号和级别计算层块的编号
#define EncodeBlockID_L(x,y,level,blockID)	(blockID) = BLOCK_ID(x,y,level)
#define DecodeBlockID_L(x,y,level,blockID)	(x) = BLOCK_ID_X(blockID);(y) = BLOCK_ID_Y(blockID);(level) = BLOCK_ID_LEVEL(blockID)
	//获取服层块的编号
#define GetParentBlockID(blockID,parentBlockID)	\
	{	\
	INT __x,__y,__level;\
	DecodeBlockID_L(__x,__y,__level,blockID);\
	__x>>=1;__y>>=1;__level--;\
	EncodeBlockID_L(__x,__y,__level,parentBlockID);\
}
//矩形框范围
typedef struct _GEO_ENVELOPE
{
	double minx;double miny;double maxx;double maxy;
	_GEO_ENVELOPE()
	{
		maxx = -180;
		maxy = -90;
		minx = 180;
		miny = 90;
	}
}ENVELOPE,GEO_RECT;
//瓦片计算工具类
class SilenTileUtils
{
public :
	//根据分辨率获取当前最靠近的级别
	static double GetLevelByDis(double dis);		
	//获取最近的一个级别根据分辨率
	static long GetNearestLevelByDis(double dis);
	//根据级别计算当前级别的宽度
	static double GetDisByLevel(int level);
	//获取下一级别的编号
	static void GetLowIds(__int64 pid,__int64 *bid);
	//
	static long GetIndex(double coord,int level,int gridSize);
	//根据地理经纬度坐标和级别计算层块的行列号
	static long GetIndex(double coord,int level);
	//根据层块左上角地理坐标计算层块编号
	static LONGLONG GetTileId(double coordx,double coordy,int level);
	//根据行列号和级别行计算地理矩形坐标范围
	static ENVELOPE GetRectByIndex(long xIndex,long yIndex,int level,int gridSize);
	////根据行列号和级别行计算经纬度地理矩形坐标范围
	static ENVELOPE GetRectByIndex(long xIndex,long yIndex,int level);
	//根据编号计算层块的地理经纬度矩形框坐标
	static ENVELOPE GetGeoRectById(LONGLONG tileId);
	//计算地理范围
	static ENVELOPE GetGeoRectById(LONGLONG tileId,int gridSize);
	//根据级别计算坐标值
	static double GetCoord(long indx,int level);
	//根据行列号计算地理坐标
	static double GetCoord(long indx,int level,int gridSize);
	//根据行列号计算地理坐标
	static void GetBlockRect(long x,long y,int level,ENVELOPE& extent);
	//根据行列号计算瓦片范围
	static void GetBlockRect(long x,long y,int level,QgsRectangle& rect);
	//计算一行的宽度
	static double GetLevelWidth(int level);
};
//
struct Tile {
  int x, y, z;
  Tile() : x(0), y(0), z(0)  {}
  Tile(int x_, int y_, int z_) : x(x_), y(y_), z(z_)  {}
  QgsPointXY toPoint() const;
  QgsRectangle toRect() const;
  long tileId() const;
  //将瓦片的左下角，按林地一张图的划分规则进行计算
  QgsPointXY toXiPoint()const;
  QgsRectangle toXiRect() const;
};
//合并的瓦片，由10*10的瓦片单元格组成，即像素单位为2560*2560
struct CombindTile{
	int minTileX;
	int minTileY;
	int maxTileX;
	int maxTileY;
	CombindTile() : minTileX(0), minTileY(0), maxTileX(0) ,maxTileY(0) {}
};
#endif
