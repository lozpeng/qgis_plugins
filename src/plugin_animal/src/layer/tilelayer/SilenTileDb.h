#ifndef SILEN_TILE_DB_H_
#define SILEN_TILE_DB_H_

#include <qobject.h>
#include "tile.h"
#include <QSqlDatabase>

typedef struct {
	LONGLONG id;
	QByteArray bytes;
}SaveBlockStru;

/**

*/
typedef struct SilenTileInfo{
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
	SilenTileInfo()
	{
		maxLevel =14;
		minLevel =0;
		tileSize =256;
		tileType = "PNG";
		tileVersion = 3;
		tileIsProj = 0;
	}
	QString getDefaultName(){
		return QString("%1_%2_%3").arg(tileThem,tileGBCode,tileYear);
	}
}SilenTileInfo;

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

//瓦片数据存储数据库
class SilenTileDb:public QObject
{
	Q_OBJECT
	public:
	SilenTileDb(QString dbPath,const EV_DB_Header& tileInfo,bool CreateNew = true);
	~SilenTileDb();
	//关闭瓦片存储数据库
	bool close();
	//保存瓦片数据
	bool save(LONGLONG tileId,QByteArray td);
	bool save(QList<SaveBlockStru> *blocks);//批量保存
	//根据瓦片编号从数据库中删除一个瓦片
	bool del(LONGLONG tileId);
	//向瓦片数据库中保存瓦片划分信息
	bool SaveSysTile(const EV_DB_Header &tileInfo);
	bool SaveDataInfo(QString myear, QString mgdcode, QString mname, QString malias,QString mzt);
	bool ExitTile(LONGLONG tileId);
	QByteArray read(LONGLONG tileId);
	bool DelSysTile();


	bool IsConnection();
	//开启数据库写入事务
	bool startTransaction();
	//关闭并提交事务
	bool endTransaction();
private:
	QString mDbPath;
	QSqlDatabase m_database;//数据库
	EV_DB_Header mTileInfo;
	int	mblock_count;
	void initdb(bool CreateNew);
	void SetDBParams();
	void ExeSQL(QString sql);
};

#endif