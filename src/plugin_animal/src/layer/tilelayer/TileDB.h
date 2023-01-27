#pragma once

#include <QString>
#include "tile.h"
#include "ReadTileDb.h"
#include "HttpFun.h"

#include <map>
#include <vector>



class CTileDB :  public QObject
{
	Q_OBJECT

public:
	QString m_connStr;

public:
	CTileDB(QObject *parent);
	virtual ~CTileDB(void);

	virtual char GetType() =0;
	virtual void close() = 0;
	//���ݵĳ�ʼ������
	virtual bool InitConnection(QString connStr) =0;
	virtual bool IsConnection() = 0;
	virtual QByteArray ReadHead() = 0;
	virtual QByteArray ReadBlock(LONGLONG id)=0;

	virtual void RequestHead()=0;
	virtual void RequestBlock(LONGLONG id)=0;
	virtual QString GetURL(LONGLONG id)=0;
};

//////////////////////////////////////
class CEDOMTileDB : public CTileDB //����edom�ļ�
{
private:
	CReadTileDb		*m_db;

public:
	CEDOMTileDB(QObject *parent);
	virtual ~CEDOMTileDB();
	
	char GetType() { return 1;};
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

//////////////////////////////////////
class CMDOMTileDB : public CTileDB //����ī����ͶӰ���ļ�
{
private:
	CReadTileDb		*m_db;

public:
	CMDOMTileDB(QObject *parent);
	virtual ~CMDOMTileDB();

	char GetType() { return 2; };
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

//////////////////////////////////////
//�׾�����
class CEviaServerTileDB : public CTileDB
{
	Q_OBJECT

private:
	QMap<LONGLONG,QByteArray>	m_DataList;

public:
	CEviaServerTileDB(QObject *parent);
	virtual ~CEviaServerTileDB();

	char GetType() { return 2;};
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
    void signal_requestHeaderFinished(bool bSuccess,const QByteArray& strResult); //http�������
	void signal_requestBlockFinished(bool bSuccess,LONGLONG tid, const QByteArray &data);

private slots:
	void slot_requestFinished(bool bSuccess,char type,LONGLONG tid, const QByteArray &data);

};


//////////////////////////////////////
class CTDServerTileDB : public CTileDB//���ͼ����
{
public:
	CTDServerTileDB(QObject *parent);
	virtual ~CTDServerTileDB();

	char GetType() { return 3;};
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
	QString GetTDBaseUrl( int downloadlevel, int i, int j);

};

////////////////////////////////////////////////////////
//���ֵķ����ַ
class CCHServerTileDB : public CTileDB//���ͼ����
{
private:
	QString		m_UserID;//�û���
	QString		m_Password;//����
public:
	CCHServerTileDB(QObject *parent);
	virtual ~CCHServerTileDB();

	void SetUSerID(QString user, QString ps) { m_UserID = user; m_Password = ps; };
	char GetType() { return 4; };
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
	QString GetCHBaseUrl(int downloadlevel, int i, int j);

};

