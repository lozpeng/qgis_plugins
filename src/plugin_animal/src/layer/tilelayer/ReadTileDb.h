#pragma once

#include <qobject.h>
#include "tile.h"
#include "SilenTileDb.h"
#include <QSqlDatabase>

//��ȡDB��
class CReadTileDb:public QObject
{
	Q_OBJECT

public:
	CReadTileDb(void);
	~CReadTileDb(void);

	bool open(QString dbPath);
	QByteArray read(LONGLONG tileId);
	//�ر���Ƭ�洢���ݿ�
	bool close();
	bool IsConnection();
	bool ReadHeader(EV_DB_Header& tileInfo);
	QByteArray ReadHeaderData();

private:
	QString mDbPath;
	EV_DB_Header mTileInfo;
	int	mblock_count;

	QSqlDatabase m_database;//���ݿ�
};

//��дȡDB��
class CWriteTileDb :public QObject
{
	Q_OBJECT

public:
	CWriteTileDb(void);
	~CWriteTileDb(void);

	bool open(QString dbPath);
	QByteArray read(LONGLONG tileId);
	//�ر���Ƭ�洢���ݿ�
	bool close();
	bool IsConnection();
	bool ReadHeader(EV_DB_Header& tileInfo);
	QByteArray ReadHeaderData();
	//������Ƭ����
	bool save(LONGLONG tileId, QByteArray td);
	bool UpdateData(LONGLONG tileId, QByteArray td);
	//������Ƭ��Ŵ����ݿ���ɾ��һ����Ƭ
	bool del(LONGLONG tileId);
	bool delDatas(LONGLONG *tileId,int count);
	bool ExitTile(LONGLONG tileId);
	bool SaveSysTile(const EV_DB_Header &tileInfo);
	bool DelSysTile();
private:
	QString mDbPath;
	EV_DB_Header mTileInfo;
	int	mblock_count;

	QSqlDatabase m_database;//���ݿ�
	void SetDBParams();
	void ExeSQL(QString sql);
};
