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
	��Ƭ���ݶ�ȡ
	*/
	class qTileDbPrivateReader : public QObject
	{
		Q_OBJECT

	public:
		qTileDbPrivateReader(void);
		~qTileDbPrivateReader(void);
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
	/**
		��Ƭ ����д��
	**/
	class qTileDbPrivateWriter : public QObject
	{
		Q_OBJECT

	public:
		qTileDbPrivateWriter(void);
		~qTileDbPrivateWriter(void);
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
		bool delDatas(LONGLONG* tileId, int count);
		bool ExitTile(LONGLONG tileId);
		bool SaveSysTile(const EV_DB_Header& tileInfo);
		bool DelSysTile();
	private:
		QString mDbPath;
		EV_DB_Header mTileInfo;
		int	mblock_count;

		QSqlDatabase m_database;//���ݿ�
		void SetDBParams();
		void ExeSQL(QString sql);
	};
}
