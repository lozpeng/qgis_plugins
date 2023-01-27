#ifndef	_GEOMETRY_H_
#define	_GEOMETRY_H_


#include <qstring.h>
#include <QgsRectangle.h>
#include <QVector>
#include <qgsgeometry.h>

/////////////////////////////////////////////////////
class CGeometry
{
protected:
	QgsRectangle	m_bound;
public:
	long		m_id;
public:
	CGeometry(void);
	virtual ~CGeometry(void);

	virtual char GetType() { return 0;};
	virtual void UpdateBound()=0;
};


class CGeoPoint : public CGeometry
{
public: 
	QVector<QgsPointXY>		m_points;
	QString	m_Name;
	QString	m_TopName,m_BottomName;

public:
	CGeoPoint();
	~CGeoPoint();

	char GetType() { return 'P';};
	void UpdateBound();
	QgsRectangle GetBound();
};

class CGeoLine : public CGeometry
{
public:
	QVector<CGeoPoint>	m_line;
public:
	CGeoLine();
	~CGeoLine();

	char GetType() { return 'L';};
	void UpdateBound();
	QgsRectangle GetBound();
};

class CGeoPolygon : public CGeometry
{
public:
	QVector<CGeoPoint>	m_polyon;
public:
	CGeoPolygon();
	~CGeoPolygon();

	char GetType() { return 'R';};
	void UpdateBound();
	QgsRectangle *GetBound();

	bool InPolygon(double x, double y);
	int PolygonInPolygon(CGeoPolygon *polygon);
	bool PolylineInPolygon(CGeoLine *line);
	//!从qgis 的geometry转换而来
	bool fromQgsGeometry(QgsGeometry qgsGeometry);
private:
	//线段是否相交或者线段是否在面里面
	bool LineIntersectPolygon(QgsPointXY s, QgsPointXY e);

};

///////////////////////////////////////////////////////
#endif //_GEOMETRY_H_
