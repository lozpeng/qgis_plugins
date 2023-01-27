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
	QString tileName; //�������
	QString aliasName; //������
	QString tileThem; //���ר��
	int tileSize;		//����С 256��С
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
	QString tileType; //��ƬͼƬ����
	QString tileDbPath;//��Ƭ���ݿ�·��
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

	long factor;//�ڲ��ߴ�/ʵ�ʳߴ�
	float lowvalue;
	float highvalue;
	double dfactor;

	
	_EV_UNITE_DB_Header()
	{
		size = sizeof(_EV_UNITE_DB_Header);
		version = 4;
		dbtype = 0;				//0 ΪDOM   1ΪDEM 2Ϊ���߶�����,=3Ϊī����ͶӰ����ͼ����
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

//��Ƭ���ݴ洢���ݿ�
class SilenTileDb:public QObject
{
	Q_OBJECT
	public:
	SilenTileDb(QString dbPath,const EV_DB_Header& tileInfo,bool CreateNew = true);
	~SilenTileDb();
	//�ر���Ƭ�洢���ݿ�
	bool close();
	//������Ƭ����
	bool save(LONGLONG tileId,QByteArray td);
	bool save(QList<SaveBlockStru> *blocks);//��������
	//������Ƭ��Ŵ����ݿ���ɾ��һ����Ƭ
	bool del(LONGLONG tileId);
	//����Ƭ���ݿ��б�����Ƭ������Ϣ
	bool SaveSysTile(const EV_DB_Header &tileInfo);
	bool SaveDataInfo(QString myear, QString mgdcode, QString mname, QString malias,QString mzt);
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

#endif