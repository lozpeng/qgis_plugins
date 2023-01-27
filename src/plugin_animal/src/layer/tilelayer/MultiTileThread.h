#pragma once
#include <QFileInfo>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QThread>
#include <QTime>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlRecord>
#include <QVariant>

#include <qgscoordinatetransform.h>
#include <qgsmapcanvas.h>
#include <qgsrectangle.h>
#include <QgsGeometry.h>

#include "SilenTileDb.h"
#include "tile.h"
#include "geometry.h"
#include "ReadTileDb.h"

typedef struct{
	QgsRectangle outRect;
	int minx;
	int maxx;
	int miny;
	int maxy;

	int level;
	int minIndX;
	int minIndY;
	int maxIndX;
	int maxIndY;
}CutTileStru;


class CTileUtils
{
public:
	CTileUtils();
	virtual ~CTileUtils();

	virtual int GetType() = 0;
	virtual long GetIndex_X(double pos, int level) = 0;
	virtual long GetIndex_Y(double pos, int level) = 0;
	//���ݼ����������ֵ
	virtual double GetCoord_X(long indx, int level) = 0;
	virtual double GetCoord_Y(long indx, int level) = 0;
	//�������кż����������
	virtual void GetBlockRect(long x, long y, int level, ENVELOPE& extent) = 0;
	//�������кż�����Ƭ��Χ
	virtual void GetBlockRect(long x, long y, int level, QgsRectangle& rect) = 0;
	//����һ�еĿ��
	virtual double GetLevelWidth(int level) = 0;
	virtual double GetRefValue() = 0;
};

//��γ��
class CEDomUtils : public CTileUtils
{
public:
	CEDomUtils();
	virtual  ~CEDomUtils();

	int GetType() { return 1; };
	long GetIndex_X(double pos, int level);
	long GetIndex_Y(double pos, int level);
	//���ݼ����������ֵ
	double GetCoord_X(long indx, int level);
	double GetCoord_Y(long indx, int level);
	//�������кż����������
	void GetBlockRect(long x, long y, int level, ENVELOPE& extent);
	//�������кż�����Ƭ��Χ
	void GetBlockRect(long x, long y, int level, QgsRectangle& rect);
	//����һ�еĿ��
	double GetLevelWidth(int level);
	double GetRefValue() { return 90.0; };
};

//ī����ͶӰ
class CMercatorUtils : public CTileUtils
{
public:
	CMercatorUtils();
	virtual  ~CMercatorUtils();

	int GetType() { return 1; };
	long GetIndex_X(double pos, int level);
	long GetIndex_Y(double pos, int level);
	//���ݼ����������ֵ
	double GetCoord_X(long indx, int level);
	double GetCoord_Y(long indx, int level);
	//�������кż����������
	void GetBlockRect(long x, long y, int level, ENVELOPE& extent);
	//�������кż�����Ƭ��Χ
	void GetBlockRect(long x, long y, int level, QgsRectangle& rect);
	//����һ�еĿ��
	double GetLevelWidth(int level);
	double GetRefValue();
};

class CCutEDOMThread;
//����㹫ʽ����Ҫ���ж϶���ι�ϵ��ͼƬ����
class CBlockThread : public QThread
{
	Q_OBJECT
public:
	CBlockThread(CCutEDOMThread *pCutEdom,QObject* parent);
	~CBlockThread();

	void Init(CGeoPolygon *DataBoundPolygon,CEDomUtils *pUtils, EV_DB_Header *header, CWriteTileDb	*TileStore);
	bool AddData(LONGLONG tid);
	bool IsProcessging() { return mDataValid; };
	void Fini();
private:
	CCutEDOMThread *m_pCutEdom;
	CGeoPolygon *m_pDataBoundPolygon;
	LONGLONG	m_tid;
	CEDomUtils	*m_pUtils;
	EV_DB_Header *m_header;
	CWriteTileDb	*mTileStore;
	bool		mDataValid;//�����Ƿ�ʼ��Ч
	bool		mStopThread;
	int Block_Polygon(CGeoPolygon *pDataBoundPolygon, LONGLONG tileId, CEDomUtils *pTileUtils);
	void ProcessBlockData(CGeoPolygon *pDataBoundPolygon,
		LONGLONG tileId, CEDomUtils *pTileUtils, EV_DB_Header *header);
	void CutImage(CGeoPolygon *pDataBoundPolygon, LONGLONG tileId, CEDomUtils *pTileUtils, QImage *outmap);
	void DelBlockData(LONGLONG tileId, EV_DB_Header *header);
	void DelChildBlockData(LONGLONG tileId, EV_DB_Header *header);
protected:
	void run();

};
//////////////////////////////////////////////////
//
class CCutEDOMThread : public QThread
{
	Q_OBJECT
public:
	CCutEDOMThread(QString edompath, CGeoPolygon *DataBoundPolygon, int ProcessThreadCount,QObject* parent);
	~CCutEDOMThread();

	void UnitBlock(CWriteTileDb *pTileDb,LONGLONG tid, QImage bk_image);//������Ҫ�ϲ������ݿ�
	void DelBlock(CWriteTileDb *pTileDb, LONGLONG tid);//�����Ҫɾ�������ݿ�
	bool ExitTile(CWriteTileDb *pTileDb, LONGLONG tid);
private:
	int				m_ProcessThreadCount;
	QString			m_EdomPath;
	CGeoPolygon		*m_pDataBoundPolygon;//���ݵı߽緶Χ��ֻ����������,���ڲü�ʹ��
	bool			mStop;
	QList<CBlockThread*>	m_BlockThreads;
	QList<LONGLONG>			m_UnitTIDs, m_DelTIDs;
	QList<QImage>			m_UnitImages;
	QMutex					mBlockDBMutex;//��CWriteTileDb����������

	void DelBlockData(CWriteTileDb *pTileDb,bool all=false);
	void UnitImage(LONGLONG tileId, CWriteTileDb *pTileDb,QImage *bk_map);
protected:
	void run();
signals:
	void signalProcess(int percent);
	void signalFinish();
	void signalMessage(QString msg);

public slots:
	void on_stop() { mStop = true; }

};
//////////////////////////////////////////////////

class CMultiTileThread;
//���ݿ鴦���߳�
//һ���������̣߳���Ⱦ���ݿ飬Ȼ�����
class CCutTileThread : public QThread
{
	Q_OBJECT
	QPainter mpainter;
	QgsMapSettings mSettings,mAlphaSettings;//mAlphaSettingsΪalpahͼ�����
	QImage mPixmap,mAlphaPixmap;
	CMultiTileThread *m_Parent;
private:
	bool	m_bRun;
	bool	m_working;//�������߳��Ƿ��ڹ���
	QList<CutTileStru>		mOutRects;//��Ҫ��������ݿ鷶Χ
	QMutex					mOutRectMutex;//mOutRects����������
	CTileUtils				*m_pTileUtils;//��ǰ��ļ��㹫ʽ
	void InitAlphaSettings();
	void InitAlphaData(QList<QgsMapLayer*> &Layers);
	void AddTreeGroup(QString GroupName, QList<QgsMapLayer*> &Layers);
	void AddMapLayer(QString LayerName, QList<QgsMapLayer*> &Layers);


	void ShapeImage(CGeoPolygon *pDataBoundPolygon, LONGLONG tileId, QImage *outmap);
	bool IsValid(CGeoPolygon *pDataBoundPolygon, LONGLONG tileId);
	void saveTileData(LONGLONG tileId,QgsRectangle MapRect,QImage &pixMap);
	void KillThread() { m_bRun = false; };//kill�̣߳������ݴ�����ɺ�������
	void BeginWork() { m_working = true;};
	void EndWork() { m_working = false;};
protected:
	void run();

public:
  CCutTileThread(CTileUtils *utils,QObject* parent);

  bool working() { return m_working;};
  void Init(CMultiTileThread *parent, QgsMapCanvas* ifaceCanvas);
  void Fini();

  ~CCutTileThread();

  bool AddData(QgsRectangle outRect,int minx,int miny,int maxx,int maxy,
				int level,int minIndX,int minIndY,int maxIndX,int maxIndY);
  int GetListCount() { return mOutRects.size(); };
};
////////////////////////////////////////////////////////////////
class CMultiTileThread :	public QThread
{
	Q_OBJECT
	EV_DB_Header mTileInfo;
	SilenTileDb* mTileStore;
	QString mtileDbPath;//��Ƭ���ݿ�·��
	QList<CCutTileThread*>	m_CutTileThreads;
	
	QgsCoordinateTransform* mTransform;//�������Ƭ��Χ���ͼ���ݷ�Χת��
	bool mStop;
	bool mIsCreateNewFile;

	void RenderBlockByLevel(int level);
	void saveTileData(LONGLONG tileId,QgsRectangle MapRect,QPixmap &pixMap);
public:
	bool	m_bCompressTile;//�Ƿ�ѹ��ͼƬ��=TRUE��ʾҪѹ��ͼƬ��Ĭ��Ϊfalse
	QString	m_AlphaTreeGroupName;//͸����ͼ�����ƣ�Ϊ�ձ�ʾ����Ҫ
private:
	QObject		*m_pObject;
	QList<SaveBlockStru>	m_SaveList;//��Ҫ��������ݿ�����
	QMutex					mSaveListMutex;//��m_SaveList����������
	CGeoPolygon		*m_pDataBoundPolygon;//���ݵı߽緶Χ��ֻ����������,���ڲü�ʹ��
	CTileUtils		*m_pTileUtils;
	QList<QgsRectangle>		m_ValidateBound;//���ݵ���Ч����Χ

	void CreateCutTileThread(QObject* parent,QgsMapCanvas* ifaceCanvas);
	void FreeCutTileThread();
	bool AddThread(QgsRectangle outRect,int minx,int miny,int maxx,int maxy,
			int level,int minIndX, int minIndY, int maxIndX, int maxIndY);//�����������̣߳��ɹ�����TRUE��ʧ�ܷ���FALSE
	void SaveTileToDB(bool AllCommit);
	void ShowThreadInfo();
	void ReadBound(QString shp_file);//��ȡ��Ҫ�ü��Ķ���η�Χ

	//�������ļ�����������û����ܵ�ǰ�������ݴ�����ȵ������
	LONGLONG		m_TileCount;
	LONGLONG		m_CurTileStep;
	QString m_msg;//��Ϣ
	//////////////////////////////////////////////
	int		m_ProcessCount;//��̨�߳�����
public:
	QString myear,mgdcode,mname,malias,mzt;

	bool ConnectDBSucceed();
	void SaveData(LONGLONG id,QByteArray bytes, std::string format);
	int GetProcessCount() { return m_ProcessCount;};
	CGeoPolygon *GetBoundPolygon() {
		return m_pDataBoundPolygon;
	};
	void AddValidateBound(QgsRectangle bound) { m_ValidateBound.append(bound); };
	//�ж�ĳ��ID�Ƿ���������Ч��Χ��
	bool IDInValidateBound(ENVELOPE tileRect);
protected:
	void run();
	void saveTileData(LONGLONG id,QgsRectangle MapRect,QImage& image);

signals:
  void signalThreadPercent(int percent);
  void signalMessage(QString msg);
  void signalProcess(int percent);
  void signalFinish();
  void DataFinish(QString _t1);

public slots:
  void on_stop()  { mStop = true; }


public:
	CMultiTileThread
		( QObject* parent
		, QgsMapCanvas* ifaceCanvas
		, const EV_DB_Header& tileInfo
		, const  QString tileDbPath
		, bool CreateNewFile
		, int	ProcessNum
		, QString PolygoFilePath
		, QString AlphaName);
	~CMultiTileThread(void);
};

/////////////////////////////
class CDownloadThread : public QThread
{//�����߳�
	Q_OBJECT
public:
	CDownloadThread(QObject* parent);
	~CDownloadThread(void);

	int m_type; //=1Ϊedom����=2Ϊ���ͼ����,=4Ϊ���ң�е�ͼ
	QString m_url;
	int m_minlevel, m_maxlevel;
	EV_DB_Header	mTileInfo;
	SilenTileDb*	mTileStore;
	QString			mtileDbPath;//��Ƭ���ݿ�·��
	QString			mUserID;
	QString			mPwd;
	CGeoPolygon		mDownloadGeometry;//��Ҫ���ص�����Χ��Ϊ����η�Χ
	void stop() { mStop = true; };
protected:
	void run();

private:
	bool	mStop;

	void DownloadData(int level,int min_lx, int max_lx, int min_y, int max_y);
	QString GetTDBaseUrl(int downloadlevel, int i, int j);
	QString GetCHBaseUrl(int downloadlevel, int i, int j);
	QString GetURL(LONGLONG id);

signals:
	void signalThreadPercent(int curlevel,int percent);
	void signalMessage(QString msg);
	void signalFinish();

public slots:
	void slot_requestFinished(bool bSuccess, char type, LONGLONG tid, const QByteArray &data);
};

/////////////////////////////////////////////
//CAddTopTileThread�޲�������������ݵ��߳�
class CAddTopTileThread : public QThread
{
	Q_OBJECT
public:
	CAddTopTileThread(QObject* parent,const QString tileDbPath);
	~CAddTopTileThread(void);

private:
	EV_DB_Header mTileInfo;
	CWriteTileDb* mTileStore;
	QString mtileDbPath;//��Ƭ���ݿ�·��
	bool		mStop;

	QImage* createImage(int grid) const;

signals:
	void signalThreadPercent(int curlevel, int percent);
	void signalMessage(QString msg);
	void signalFinish();

protected:
	void run();

public slots:
	void on_stop() { mStop = true; }

};

/////////////////////////////////////////////
//�ϲ�edom���ݵ��߳�
class CUnitTileThread : public QThread
{
	Q_OBJECT
public:
	CUnitTileThread(QObject* parent, const QString tileDbPath, const QStringList edomPaths);
	~CUnitTileThread(void);

private:
	
	bool		mStop;

	QString		m_SaveDBPath;
	QStringList	m_DataList;

	void UnitData(SilenTileDb *SaveDB, EV_DB_Header *SavaHeader, QString DataFile);
	bool MergeData(QByteArray *read_bytes, QByteArray *save_bytes, QByteArray *out_bytes);
signals:
	void signalProcess(int percent);
	void signalMessage(QString msg);
	void signalFinish();

protected:
	void run();

	public slots:
	void on_stop() { mStop = true; }

};

/////////////////////////////////////////////
//�ϲ�edom���ݵ��߳�
class CTransparentThread : public QThread
{
	Q_OBJECT
public:
	CTransparentThread(QObject* parent, const QString ImgPath, const QVector<QColor> colors);
	~CTransparentThread(void);

private:

	bool		mStop;

	QString		m_ImgPath;
	QVector<QColor>	m_colors;

signals:
	void signalProcess(int percent);
	void signalMessage(QString msg);
	void signalFinish();

protected:
	void run();
	bool FindFile(const QString &path);
	void TransparentImage(QString FileName);
	bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);

public slots:
	void on_stop() { mStop = true; }

};

//DEMȥ����߳�
class CDemNoiseThread : public QThread
{
	Q_OBJECT
public:
	CDemNoiseThread(QObject* parent, const QStringList Paths, double MinValue, double MaxValue);
	~CDemNoiseThread(void);

private:
	QStringList	mPaths;
	bool		mStop;
	double		m_MinAlt, m_MaxAlt;

signals:
	void signalProcess(int percent);
	void signalMessage(QString msg);
	void signalFinish();

protected:
	void run();

public slots:
	void on_stop() { mStop = true; }

};