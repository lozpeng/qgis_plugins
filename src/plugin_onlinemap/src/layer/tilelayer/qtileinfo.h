#pragma once
#include <limits>  
using namespace std;

#include <qgspoint.h>
#include <qgsrectangle.h>
#define	OUTIMAGE_WIDTH 35 //����Ӱ��Ŀ��
#define	OUTIMAGE_HEIGHT	35 //����Ӱ���ÿ���߶�
#define	IMAGE_SIZE	256 //һ��IDӰ�����ݵĿ��

static const double LN2 = 0.693147180;
////��߼���
static const int TOP_LEVEL = 16;
//�ۺ���Ƭ��С
static const int TILE_MATRIX = 2048;
////��߼���ߴ�
//const long TOP_LEVEL_SIZE =1 << TOP_LEVEL;
////����С
//const long TILE_GRID = 256;
////���ߴ�
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
//���ݲ���ż�������к�
#define	BLOCK_ID_X(id)		GET_SIGN_28((int)((id) & __28_MASK))
	//���ݲ���ż������к�
#define	BLOCK_ID_Y(id)		GET_SIGN_28((int)(((id)>>28) & __28_MASK))

	//���ݲ���ż�����ļ���
#define	BLOCK_ID_LEVEL(id)	(int)(((id)>>56)&0xFF)
	//�������кźͼ��������ı��
#define	BLOCK_ID(x,y,level)	(((x)&__28_MASK)|((LONGLONG)((y)&__28_MASK)<<28)|((LONGLONG)(level)<<56))

	//�����кźͼ��������ı��
#define EncodeBlockID_L(x,y,level,blockID)	(blockID) = BLOCK_ID(x,y,level)
#define DecodeBlockID_L(x,y,level,blockID)	(x) = BLOCK_ID_X(blockID);(y) = BLOCK_ID_Y(blockID);(level) = BLOCK_ID_LEVEL(blockID)
	//��ȡ�����ı��
#define GetParentBlockID(blockID,parentBlockID)	\
	{	\
	INT __x,__y,__level;\
	DecodeBlockID_L(__x,__y,__level,blockID);\
	__x>>=1;__y>>=1;__level--;\
	EncodeBlockID_L(__x,__y,__level,parentBlockID);\
}
namespace geotile {
	//���ο�Χ
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
		//����Ƭ�����½ǣ����ֵ�һ��ͼ�Ļ��ֹ�����м���
		QgsPointXY toXiPoint()const;
		QgsRectangle toXiRect() const;
	};
	//�ϲ�����Ƭ����10*10����Ƭ��Ԫ����ɣ������ص�λΪ2560*2560
	struct qCombindTile {
		int minTileX;
		int minTileY;
		int maxTileX;
		int maxTileY;
		qCombindTile() : minTileX(0), minTileY(0), maxTileX(0), maxTileY(0) {}
	};
	//��Ƭ��
	typedef struct {
		LONGLONG id;
		QByteArray bytes;
	}qSaveBlockStru;
	/// <summary>
	/// ��Ƭ������Ϣ
	/// </summary>
	typedef struct qTileInfo {
		QString tileName; //�������
		QString aliasName; //������
		QString tileThem; //���ר��
		int tileSize;		//����С 256��С
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
		QString tileType; //��ƬͼƬ����
		QString tileDbPath;//��Ƭ���ݿ�·��
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
	/// ��Ƭ����ͷ��Ϣ
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

		long factor;//�ڲ��ߴ�/ʵ�ʳߴ�
		float lowvalue;
		float highvalue;
		double dfactor;


		_EV_UNITE_DB_Header()
		{
			size = sizeof(_EV_UNITE_DB_Header);
			version = 4;
			dbtype = 0;				//0 ΪDOM   1ΪDEM 2Ϊ���߶�����,=3Ϊī����ͶӰ����ͼ����
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