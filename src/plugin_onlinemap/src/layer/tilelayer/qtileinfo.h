#pragma once
#include <limits>  
using namespace std;

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
namespace geotile {
	//矩形框范围
	typedef struct _GEO_ENVELOPE
	{
		double minx; double miny; double maxx; double maxy;
		_GEO_ENVELOPE()
		{
			maxx = -180;
			maxy = -90;
			minx = 180;
			miny = 90;
		}
	}ENVELOPE, GEO_RECT;

	struct qTile {
		int x, y, z;
		qTile() : x(0), y(0), z(0) {}
		qTile(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
		QgsPointXY toPoint() const;
		QgsRectangle toRect() const;
		long tileId() const;
		//将瓦片的左下角，按林地一张图的划分规则进行计算
		QgsPointXY toXiPoint()const;
		QgsRectangle toXiRect() const;
	};
	//合并的瓦片，由10*10的瓦片单元格组成，即像素单位为2560*2560
	struct qCombindTile {
		int minTileX;
		int minTileY;
		int maxTileX;
		int maxTileY;
		qCombindTile() : minTileX(0), minTileY(0), maxTileX(0), maxTileY(0) {}
	};
	//瓦片块
	typedef struct {
		LONGLONG id;
		QByteArray bytes;
	}qSaveBlockStru;
	/// <summary>
	/// 瓦片基础信息
	/// </summary>
	typedef struct qTileInfo {
		QString tileName; //层块名称
		QString aliasName; //层块别名
		QString tileThem; //层块专题
		int tileSize;		//层块大小 256大小
		int maxLevel;
		int minLevel;
		double minX;
		double minY;
		double maxX;
		double maxY;
		QString tileYear;
		QString tileGBCode;
		int tileVersion;
		int tileIsProj;
		QString tileType; //瓦片图片类型
		QString tileDbPath;//瓦片数据库路径
		qTileInfo()
		{
			maxLevel = 14;
			minLevel = 0;
			tileSize = 256;
			tileType = "PNG";
			tileVersion = 3;
			tileIsProj = 0;
		}
		QString getDefaultName() {
			return QString("%1_%2_%3").arg(tileThem, tileGBCode, tileYear);
		}
	}_qTileInfo;
	/// <summary>
	/// 瓦片数据头信息
	/// </summary>
	typedef struct _EV_UNITE_DB_Header
	{
		unsigned short size;
		unsigned short version;

		unsigned char dbtype;
		unsigned char tileGeoSize;
		unsigned char tileGridSize;
		unsigned char jpgQuality;

		unsigned char minlevel;
		unsigned char maxlevel;

		unsigned char reserved[2];

		double minx;
		double miny;
		double maxx;
		double maxy;

		long factor;//内部尺寸/实际尺寸
		float lowvalue;
		float highvalue;
		double dfactor;


		_EV_UNITE_DB_Header()
		{
			size = sizeof(_EV_UNITE_DB_Header);
			version = 4;
			dbtype = 0;				//0 为DOM   1为DEM 2为二者都包含,=3为墨卡托投影的切图数据
			tileGeoSize = 8;
			tileGridSize = 8;
			jpgQuality = 75;
			minlevel = 0;
			maxlevel = 0;
			factor = 1;
			lowvalue = 0;
			highvalue = 0;
			dfactor = 1.0f;

			minx = 0;
			miny = 0;
			maxx = 0;
			maxy = 0;
		}
	}EV_DB_Header;
}