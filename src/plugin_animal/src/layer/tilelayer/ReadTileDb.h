#pragma once

#include <qobject.h>
#include "tile.h"
#include "SilenTileDb.h"
#include <QSqlDatabase>

//读取DB类
class CReadTileDb:public QObject
{
	Q_OBJECT

public:
	CReadTileDb(void);
	~CReadTileDb(void);

	bool open(QString dbPath);
	QByteArray read(LONGLONG tileId);
	//关闭瓦片存储数据库
	bool close();
	bool IsConnection();
	bool ReadHeader(EV_DB_Header& tileInfo);
	QByteArray ReadHeaderData();

private:
	QString mDbPath;
	EV_DB_Header mTileInfo;
	int	mblock_count;

	QSqlDatabase m_database;//数据库
};

//读写取DB类
class CWriteTileDb :public QObject
{
	Q_OBJECT

public:
	CWriteTileDb(void);
	~CWriteTileDb(void);

	bool open(QString dbPath);
	QByteArray read(LONGLONG tileId);
	//关闭瓦片存储数据库
	bool close();
	bool IsConnection();
	bool ReadHeader(EV_DB_Header& tileInfo);
	QByteArray ReadHeaderData();
	//保存瓦片数据
	bool save(LONGLONG tileId, QByteArray td);
	bool UpdateData(LONGLONG tileId, QByteArray td);
	//根据瓦片编号从数据库中删除一个瓦片
	bool del(LONGLONG tileId);
	bool delDatas(LONGLONG *tileId,int count);
	bool ExitTile(LONGLONG tileId);
	bool SaveSysTile(const EV_DB_Header &tileInfo);
	bool DelSysTile();
private:
	QString mDbPath;
	EV_DB_Header mTileInfo;
	int	mblock_count;

	QSqlDatabase m_database;//数据库
	void SetDBParams();
	void ExeSQL(QString sql);
};
