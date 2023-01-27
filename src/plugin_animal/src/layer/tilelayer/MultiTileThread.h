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
	//根据级别计算坐标值
	virtual double GetCoord_X(long indx, int level) = 0;
	virtual double GetCoord_Y(long indx, int level) = 0;
	//根据行列号计算地理坐标
	virtual void GetBlockRect(long x, long y, int level, ENVELOPE& extent) = 0;
	//根据行列号计算瓦片范围
	virtual void GetBlockRect(long x, long y, int level, QgsRectangle& rect) = 0;
	//计算一行的宽度
	virtual double GetLevelWidth(int level) = 0;
	virtual double GetRefValue() = 0;
};

//经纬度
class CEDomUtils : public CTileUtils
{
public:
	CEDomUtils();
	virtual  ~CEDomUtils();

	int GetType() { return 1; };
	long GetIndex_X(double pos, int level);
	long GetIndex_Y(double pos, int level);
	//根据级别计算坐标值
	double GetCoord_X(long indx, int level);
	double GetCoord_Y(long indx, int level);
	//根据行列号计算地理坐标
	void GetBlockRect(long x, long y, int level, ENVELOPE& extent);
	//根据行列号计算瓦片范围
	void GetBlockRect(long x, long y, int level, QgsRectangle& rect);
	//计算一行的宽度
	double GetLevelWidth(int level);
	double GetRefValue() { return 90.0; };
};

//墨卡托投影
class CMercatorUtils : public CTileUtils
{
public:
	CMercatorUtils();
	virtual  ~CMercatorUtils();

	int GetType() { return 1; };
	long GetIndex_X(double pos, int level);
	long GetIndex_Y(double pos, int level);
	//根据级别计算坐标值
	double GetCoord_X(long indx, int level);
	double GetCoord_Y(long indx, int level);
	//根据行列号计算地理坐标
	void GetBlockRect(long x, long y, int level, ENVELOPE& extent);
	//根据行列号计算瓦片范围
	void GetBlockRect(long x, long y, int level, QgsRectangle& rect);
	//计算一行的宽度
	double GetLevelWidth(int level);
	double GetRefValue();
};

class CCutEDOMThread;
//块计算公式，主要是判断多边形关系和图片处理
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
	bool		mDataValid;//数据是否开始有效
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

	void UnitBlock(CWriteTileDb *pTileDb,LONGLONG tid, QImage bk_image);//加入需要合并的数据块
	void DelBlock(CWriteTileDb *pTileDb, LONGLONG tid);//添加需要删除的数据块
	bool ExitTile(CWriteTileDb *pTileDb, LONGLONG tid);
private:
	int				m_ProcessThreadCount;
	QString			m_EdomPath;
	CGeoPolygon		*m_pDataBoundPolygon;//数据的边界范围，只能是面数据,用于裁剪使用
	bool			mStop;
	QList<CBlockThread*>	m_BlockThreads;
	QList<LONGLONG>			m_UnitTIDs, m_DelTIDs;
	QList<QImage>			m_UnitImages;
	QMutex					mBlockDBMutex;//对CWriteTileDb操作的锁定

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
//数据块处理线程
//一个独立的线程，渲染数据块，然后输出
class CCutTileThread : public QThread
{
	Q_OBJECT
	QPainter mpainter;
	QgsMapSettings mSettings,mAlphaSettings;//mAlphaSettings为alpah图层对象
	QImage mPixmap,mAlphaPixmap;
	CMultiTileThread *m_Parent;
private:
	bool	m_bRun;
	bool	m_working;//表明本线程是否在工作
	QList<CutTileStru>		mOutRects;//需要输出的数据块范围
	QMutex					mOutRectMutex;//mOutRects变量的锁定
	CTileUtils				*m_pTileUtils;//当前块的计算公式
	void InitAlphaSettings();
	void InitAlphaData(QList<QgsMapLayer*> &Layers);
	void AddTreeGroup(QString GroupName, QList<QgsMapLayer*> &Layers);
	void AddMapLayer(QString LayerName, QList<QgsMapLayer*> &Layers);


	void ShapeImage(CGeoPolygon *pDataBoundPolygon, LONGLONG tileId, QImage *outmap);
	bool IsValid(CGeoPolygon *pDataBoundPolygon, LONGLONG tileId);
	void saveTileData(LONGLONG tileId,QgsRectangle MapRect,QImage &pixMap);
	void KillThread() { m_bRun = false; };//kill线程，在数据处理完成后再运行
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
	QString mtileDbPath;//瓦片数据库路径
	QList<CCutTileThread*>	m_CutTileThreads;
	
	QgsCoordinateTransform* mTransform;//计算的瓦片范围向地图数据范围转换
	bool mStop;
	bool mIsCreateNewFile;

	void RenderBlockByLevel(int level);
	void saveTileData(LONGLONG tileId,QgsRectangle MapRect,QPixmap &pixMap);
public:
	bool	m_bCompressTile;//是否压缩图片，=TRUE表示要压缩图片，默认为false
	QString	m_AlphaTreeGroupName;//透明的图层名称，为空表示不需要
private:
	QObject		*m_pObject;
	QList<SaveBlockStru>	m_SaveList;//需要保存的数据块序列
	QMutex					mSaveListMutex;//对m_SaveList操作的锁定
	CGeoPolygon		*m_pDataBoundPolygon;//数据的边界范围，只能是面数据,用于裁剪使用
	CTileUtils		*m_pTileUtils;
	QList<QgsRectangle>		m_ValidateBound;//数据的有效区域范围

	void CreateCutTileThread(QObject* parent,QgsMapCanvas* ifaceCanvas);
	void FreeCutTileThread();
	bool AddThread(QgsRectangle outRect,int minx,int miny,int maxx,int maxy,
			int level,int minIndX, int minIndY, int maxIndX, int maxIndY);//将工作加入线程，成果返回TRUE，失败返回FALSE
	void SaveTileToDB(bool AllCommit);
	void ShowThreadInfo();
	void ReadBound(QString shp_file);//读取需要裁剪的多边形范围

	//进度条的计算变量，让用户感受当前进行数据处理进度的情况。
	LONGLONG		m_TileCount;
	LONGLONG		m_CurTileStep;
	QString m_msg;//消息
	//////////////////////////////////////////////
	int		m_ProcessCount;//后台线程数量
public:
	QString myear,mgdcode,mname,malias,mzt;

	bool ConnectDBSucceed();
	void SaveData(LONGLONG id,QByteArray bytes, std::string format);
	int GetProcessCount() { return m_ProcessCount;};
	CGeoPolygon *GetBoundPolygon() {
		return m_pDataBoundPolygon;
	};
	void AddValidateBound(QgsRectangle bound) { m_ValidateBound.append(bound); };
	//判断某个ID是否在区域有效范围内
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
{//下载线程
	Q_OBJECT
public:
	CDownloadThread(QObject* parent);
	~CDownloadThread(void);

	int m_type; //=1为edom服务，=2为天地图服务,=4为测绘遥感地图
	QString m_url;
	int m_minlevel, m_maxlevel;
	EV_DB_Header	mTileInfo;
	SilenTileDb*	mTileStore;
	QString			mtileDbPath;//瓦片数据库路径
	QString			mUserID;
	QString			mPwd;
	CGeoPolygon		mDownloadGeometry;//需要下载的区域范围，为多边形范围
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
//CAddTopTileThread修补顶层金字塔数据的线程
class CAddTopTileThread : public QThread
{
	Q_OBJECT
public:
	CAddTopTileThread(QObject* parent,const QString tileDbPath);
	~CAddTopTileThread(void);

private:
	EV_DB_Header mTileInfo;
	CWriteTileDb* mTileStore;
	QString mtileDbPath;//瓦片数据库路径
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
//合并edom数据的线程
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
//合并edom数据的线程
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

//DEM去噪点线程
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