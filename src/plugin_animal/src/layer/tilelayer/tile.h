#ifndef TILE_H
#define TILE_H
#include <limits>  
 

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
//���ο�Χ
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
//��Ƭ���㹤����
class SilenTileUtils
{
public :
	//���ݷֱ��ʻ�ȡ��ǰ����ļ���
	static double GetLevelByDis(double dis);		
	//��ȡ�����һ��������ݷֱ���
	static long GetNearestLevelByDis(double dis);
	//���ݼ�����㵱ǰ����Ŀ��
	static double GetDisByLevel(int level);
	//��ȡ��һ����ı��
	static void GetLowIds(__int64 pid,__int64 *bid);
	//
	static long GetIndex(double coord,int level,int gridSize);
	//���ݵ���γ������ͼ������������к�
	static long GetIndex(double coord,int level);
	//���ݲ�����Ͻǵ��������������
	static LONGLONG GetTileId(double coordx,double coordy,int level);
	//�������кźͼ����м������������귶Χ
	static ENVELOPE GetRectByIndex(long xIndex,long yIndex,int level,int gridSize);
	////�������кźͼ����м��㾭γ�ȵ���������귶Χ
	static ENVELOPE GetRectByIndex(long xIndex,long yIndex,int level);
	//���ݱ�ż�����ĵ���γ�Ⱦ��ο�����
	static ENVELOPE GetGeoRectById(LONGLONG tileId);
	//�������Χ
	static ENVELOPE GetGeoRectById(LONGLONG tileId,int gridSize);
	//���ݼ����������ֵ
	static double GetCoord(long indx,int level);
	//�������кż����������
	static double GetCoord(long indx,int level,int gridSize);
	//�������кż����������
	static void GetBlockRect(long x,long y,int level,ENVELOPE& extent);
	//�������кż�����Ƭ��Χ
	static void GetBlockRect(long x,long y,int level,QgsRectangle& rect);
	//����һ�еĿ��
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
  //����Ƭ�����½ǣ����ֵ�һ��ͼ�Ļ��ֹ�����м���
  QgsPointXY toXiPoint()const;
  QgsRectangle toXiRect() const;
};
//�ϲ�����Ƭ����10*10����Ƭ��Ԫ����ɣ������ص�λΪ2560*2560
struct CombindTile{
	int minTileX;
	int minTileY;
	int maxTileX;
	int maxTileY;
	CombindTile() : minTileX(0), minTileY(0), maxTileX(0) ,maxTileY(0) {}
};
#endif
