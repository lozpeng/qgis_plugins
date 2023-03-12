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
		//�ر���Ƭ�洢���ݿ�
		bool close();
		//������Ƭ����
		bool save(LONGLONG tileId, QByteArray td);
		bool save(QList<qSaveBlockStru>* blocks);//��������
		//������Ƭ��Ŵ����ݿ���ɾ��һ����Ƭ
		bool del(LONGLONG tileId);
		//����Ƭ���ݿ��б�����Ƭ������Ϣ
		bool SaveSysTile(const EV_DB_Header& tileInfo);
		bool SaveDataInfo(QString myear, QString mgdcode, QString mname, QString malias, QString mzt);
		bool ExitTile(LONGLONG tileId);
		QByteArray read(LONGLONG tileId);
		bool DelSysTile();

		bool IsConnection();
		//�������ݿ�д������
		bool startTransaction();
		//�رղ��ύ����
		bool endTransaction();
	private:
		QString mDbPath;
		QSqlDatabase m_database;//���ݿ�
		EV_DB_Header mTileInfo;
		int	mblock_count;
		void initdb(bool CreateNew);
		void SetDBParams();
		void ExeSQL(QString sql);
	};
}
