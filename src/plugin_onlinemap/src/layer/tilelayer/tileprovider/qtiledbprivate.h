#pragma once

#include <QObject>
#include <QSqlDatabase>

#include "../qtileinfo.h"

#include <QObject>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlRecord>
#include <QFile>
#include <QMessageBox>
#include "../xfileio.h"

namespace geotile {
	/*
	瓦片数据读取
	*/
	class qTileDbPrivateReader : public QObject
	{
		Q_OBJECT

	public:
		qTileDbPrivateReader(void);
		~qTileDbPrivateReader(void);
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
	/**
		瓦片 数据写入
	**/
	class qTileDbPrivateWriter : public QObject
	{
		Q_OBJECT

	public:
		qTileDbPrivateWriter(void);
		~qTileDbPrivateWriter(void);
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
		bool delDatas(LONGLONG* tileId, int count);
		bool ExitTile(LONGLONG tileId);
		bool SaveSysTile(const EV_DB_Header& tileInfo);
		bool DelSysTile();
	private:
		QString mDbPath;
		EV_DB_Header mTileInfo;
		int	mblock_count;

		QSqlDatabase m_database;//数据库
		void SetDBParams();
		void ExeSQL(QString sql);
	};
}
