#ifndef Q_TILE_PROVIDER_H_
#define Q_TILE_PROVIDER_H_

#include <QObject>
#include <QString>

#include <QMap>
#include <QVector>

#include "../qtileinfo.h"
#include "../qtileutils.h"

#include "qtiledbprivate.h"
namespace geotile
{
	//��Ƭ����Դ����
	enum qTileSourceType {
		tUnKnown = 0,		//δ֪��ʽ ���ܲ�������
		tEdomDb = 1,		//Evia Edom������Ƭ
		tMBTiles = 2,		//MBTiles��ʽ���ݣ�Ҳ��һ��sqlite��ʽ�����ݿ�
		tLocalDb = 3,		//�����ֲ���Ƭ���ݸ�ʽ
		tWebMecatorDb = 4, //web Mecator ��Ƭ���ݿ�
		tEdomServer = 5, //�ֲ��������߷���
		tXYZTileServer = 6,//����XYZ��Ƭ���ݸ�ʽ
		tTdtMapServer = 7, //���ͼ���ߵ�ͼ
		tCHJMapServer = 8, //���ֵ�ͼ����
	};

	/*
	* ��Ƭ���ݴ洢������
	*/
	class qTileProvider :public QObject
	{
		Q_OBJECT

	public:
		qTileProvider(QObject* parent);
		virtual ~qTileProvider(void);
		virtual qTileSourceType GetType() = 0;
		virtual void close() = 0;
		//���ݵĳ�ʼ������
		virtual bool InitConnection(QString connStr) = 0;
		virtual bool IsConnection() = 0;
		virtual QByteArray ReadHead() = 0;
		virtual QByteArray ReadBlock(LONGLONG id) = 0;

		virtual void RequestHead() = 0;
		virtual void RequestBlock(LONGLONG id) = 0;
		virtual QString GetURL(LONGLONG id) = 0;
	protected:
		QString m_connStr;// ���������ַ����������Ǳ���Ҳ�����Ƿ�������ַ
	};
	/// <summary>
	/// ����edom�ļ�
	/// </summary>
	class qEdomTileProvider : public qTileProvider
	{
	private:
		qTileDbPrivateReader* m_db;

	public:
		qEdomTileProvider(QObject* parent);
		virtual ~qEdomTileProvider();

		qTileSourceType GetType() { return qTileSourceType::tEdomDb; }
		//���ݵĳ�ʼ������
		bool InitConnection(QString connStr);
		bool IsConnection();
		void close();
		QByteArray ReadHead();
		QByteArray ReadBlock(LONGLONG id);

		void RequestHead();
		void RequestBlock(LONGLONG id);
		QString GetURL(LONGLONG id) { return ""; };
	};

	/// <summary>
	/// ����ī����ͶӰ���ļ�
	/// </summary>
	class qWebMecatorLocalDbProvider : public qTileProvider
	{
	private:
		qTileDbPrivateReader* m_db;

	public:
		qWebMecatorLocalDbProvider(QObject* parent);
		virtual ~qWebMecatorLocalDbProvider();

		qTileSourceType GetType() { return qTileSourceType::tWebMecatorDb; }
		//���ݵĳ�ʼ������
		bool InitConnection(QString connStr);
		bool IsConnection();
		void close();
		QByteArray ReadHead();
		QByteArray ReadBlock(LONGLONG id);

		void RequestHead();
		void RequestBlock(LONGLONG id);
		QString GetURL(LONGLONG id) { return ""; };
	};

	/// <summary>
	/// �׾�����
	/// </summary>
	class qEviaServerTileProvider : public qTileProvider
	{
		Q_OBJECT

	private:
		map<LONGLONG, QByteArray>	m_DataList;

	public:
		qEviaServerTileProvider(QObject* parent);
		virtual ~qEviaServerTileProvider();
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		qTileSourceType GetType() { return qTileSourceType::tEdomServer; }
		//���ݵĳ�ʼ������
		bool InitConnection(QString connStr);
		bool IsConnection();
		void close();
		QByteArray ReadHead();
		QByteArray ReadBlock(LONGLONG id);

		void RequestHead();
		void RequestBlock(LONGLONG id);
		QString GetURL(LONGLONG id);

	signals:
		void signal_requestHeaderFinished(bool bSuccess, const QByteArray& strResult); //http�������
		void signal_requestBlockFinished(bool bSuccess, LONGLONG tid, const QByteArray& data);

	private slots:
		void slot_requestFinished(bool bSuccess, char type, LONGLONG tid, const QByteArray& data);

	};
	/// <summary>
	/// ���ͼ����
	/// </summary>
	class qTdtServerTileProvider : public qTileProvider
	{
	public:
		qTdtServerTileProvider(QObject* parent);
		virtual ~qTdtServerTileProvider();

		qTileSourceType GetType() { return qTileSourceType::tTdtMapServer; }
		void close() {};
		//���ݵĳ�ʼ������
		bool InitConnection(QString connStr);

		bool IsConnection() {
			return true;
		};
		QByteArray ReadHead();
		QByteArray ReadBlock(LONGLONG id);

		void RequestHead();
		void RequestBlock(LONGLONG id);
		QString GetURL(LONGLONG id);

	private:
		QString GetTDBaseUrl(int downloadlevel, int i, int j);

	};

	/// <summary>
	/// ���ֵķ����ַ ���ͼ���� ,��Ҫ�ǶԽ�һ��ͼ
	/// </summary>
	class qCeHuiServerTileProvider : public qTileProvider
	{
	private:
		QString		m_UserID;//�û���
		QString		m_Password;//����
	public:
		qCeHuiServerTileProvider(QObject* parent);
		virtual ~qCeHuiServerTileProvider();

		void SetUSerID(QString user, QString ps) { m_UserID = user; m_Password = ps; }
		qTileSourceType GetType() { return qTileSourceType::tCHJMapServer; }
		void close() {};
		//���ݵĳ�ʼ������
		bool InitConnection(QString connStr);
		bool IsConnection() {
			return true;
		}
		QByteArray ReadHead();
		QByteArray ReadBlock(LONGLONG id);

		void RequestHead();
		void RequestBlock(LONGLONG id);
		QString GetURL(LONGLONG id);

	private:
		QString GetCHBaseUrl(int downloadlevel, int i, int j);
	};
}
#endif