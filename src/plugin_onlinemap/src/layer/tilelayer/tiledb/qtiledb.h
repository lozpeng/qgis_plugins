#pragma once

#include <QObject>
#include <QSqlDatabase>

#include "../qtileinfo.h"

namespace geotile {
	/// <summary>
	/// 
	/// </summary>
	class qTileDb : public QObject
	{
		Q_OBJECT
	public:
		qTileDb(QString dbPath, const EV_DB_Header& tileInfo, bool CreateNew = true);
		~qTileDb();
		//关闭瓦片存储数据库
		bool close();
		//保存瓦片数据
		bool save(LONGLONG tileId, QByteArray td);
		bool save(QList<qSaveBlockStru>* blocks);//批量保存
		//根据瓦片编号从数据库中删除一个瓦片
		bool del(LONGLONG tileId);
		//向瓦片数据库中保存瓦片划分信息
		bool SaveSysTile(const EV_DB_Header& tileInfo);
		bool SaveDataInfo(QString myear, QString mgdcode, QString mname, QString malias, QString mzt);
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
}
