#include "Geometry.h"
#include <qgsgeometry.h>
#include <qgspolygon.h>
CGeometry::CGeometry(void)
{
	m_id = 0;
}

CGeometry::~CGeometry(void)
{
}

/////////////////////////////////////

CGeoPoint::CGeoPoint(void)
{
	m_Name = "";
}

CGeoPoint::~CGeoPoint(void)
{
}


QgsRectangle CGeoPoint::GetBound()
{
	return m_bound;

}

void CGeoPoint::UpdateBound()
{
	if(m_points.size()<1)
		return;
	m_bound.setXMinimum(m_points[0].x());
	m_bound.setXMaximum(m_points[0].x());
	m_bound.setYMaximum(m_points[0].y());
	m_bound.setYMinimum(m_points[0].y());

	int PointCount = m_points.size();
	for(int i=1;i<PointCount;i++)
	{
		m_bound.setXMinimum(__min(m_bound.xMinimum(),m_points[i].x()));
		m_bound.setXMaximum(__max(m_bound.xMaximum(),m_points[i].x()));
		m_bound.setYMaximum(__max(m_bound.yMaximum(),m_points[i].y()));
		m_bound.setYMinimum(__min(m_bound.yMinimum(),m_points[i].y()));
	}

}
////////////////////////////////////////
CGeoLine::CGeoLine(void)
{
}

CGeoLine::~CGeoLine(void)
{
}

QgsRectangle CGeoLine::GetBound()
{
	return m_bound;
}
void CGeoLine::UpdateBound()
{
	if(m_line.size()<1)
		return;

	m_line[0].UpdateBound();
	m_bound = m_line[0].GetBound();
	QgsRectangle t;
	for(int i=1;i<m_line.size();i++)
	{
		m_line[i].UpdateBound();
		t = m_line[i].GetBound();
		m_bound.setXMinimum(__min(m_bound.xMinimum(), t.xMinimum()));
		m_bound.setXMaximum(__max(m_bound.xMaximum(), t.xMaximum()));
		m_bound.setYMaximum(__max(m_bound.yMaximum(), t.yMaximum()));
		m_bound.setYMinimum(__min(m_bound.yMinimum(), t.yMinimum()));
	}
}


////////////////////////////////////////
CGeoPolygon::CGeoPolygon(void)
{
}

CGeoPolygon::~CGeoPolygon(void)
{
}
//!从qgsGeometry对象转换为CGeoPolygon对象，要求qgsGeometry为单部件多边形
bool CGeoPolygon::fromQgsGeometry(QgsGeometry qgsGeometry)
{
	QgsPolygonXY qgsPolygon = qgsGeometry.asPolygon();
	if (qgsPolygon.isEmpty())return false;
	int size = qgsPolygon.count();
	if (size <= 0)return false;
	for (int i = 0; i < size; i++)
	{
		QgsPolylineXY line = qgsPolygon.at(i);
		CGeoPoint cGeoPoint;
		cGeoPoint.m_points.append(line);
		this->m_polyon.append(cGeoPoint);
	}
	return true;
}

QgsRectangle *CGeoPolygon::GetBound()
{
	return &m_bound;
}

void CGeoPolygon::UpdateBound()
{
	if (m_polyon.size()<1)
		return;

	m_polyon[0].UpdateBound();
	m_bound = m_polyon[0].GetBound();
	QgsRectangle t;
	for (int i = 1; i<m_polyon.size(); i++)
	{
		m_polyon[i].UpdateBound();
		t = m_polyon[i].GetBound();
		m_bound.setXMinimum(__min(m_bound.xMinimum(), t.xMinimum()));
		m_bound.setXMaximum(__max(m_bound.xMaximum(), t.xMaximum()));
		m_bound.setYMaximum(__max(m_bound.yMaximum(), t.yMaximum()));
		m_bound.setYMinimum(__min(m_bound.yMinimum(), t.yMinimum()));

	}
}

struct LINESEG
{
	QgsPointXY s;
	QgsPointXY e; LINESEG(QgsPointXY a, QgsPointXY b) { s = a; e = b; }
	LINESEG() { }
};

#define S_Vec2Geo S_Point
/******************************************************************************
r=multiply(sp,ep,op),得到(sp-op)*(ep-op)的叉积
r>0:ep在矢量opsp的逆时针方向；
r=0：opspep三点共线；
r<0:ep在矢量opsp的顺时针方向
*******************************************************************************/
double multiply(QgsPointXY sp, QgsPointXY ep, QgsPointXY op)
{
	return((sp.x() - op.x())*(ep.y() - op.y()) - (ep.x() - op.x())*(sp.y() - op.y()));
}

/* 判断点p是否在线段l上，条件：(p在线段l所在的直线上)&& (点p在以线段l为对角线的矩形内) */
bool online(LINESEG l, QgsPointXY p)
{
	return((multiply(l.e, p, l.s) == 0)
		&& (((p.x() - l.s.x())*(p.x() - l.e.x()) <= 0) && ((p.y() - l.s.y())*(p.y() - l.e.y()) <= 0)));
}

// 如果线段u和v相交(包括相交在端点处)时，返回true 
bool intersect(LINESEG u, LINESEG v)
{
	return((__max(u.s.x(), u.e.x()) >= __min(v.s.x(), v.e.x())) &&                     //排斥实验 
		(__max(v.s.x(), v.e.x()) >= __min(u.s.x(), u.e.x())) &&
		(__max(u.s.y(), u.e.y()) >= __min(v.s.y(), v.e.y())) &&
		(__max(v.s.y(), v.e.y()) >= __min(u.s.y(), u.e.y())) &&
		(multiply(v.s, u.e, u.s)*multiply(u.e, v.e, u.s) >= 0) &&         //跨立实验 
		(multiply(u.s, v.e, v.s)*multiply(v.e, u.e, v.s) >= 0));
}

//  (线段u和v相交)&&(交点不是双方的端点) 时返回true    
bool intersect_A(LINESEG u, LINESEG v)
{
	return((intersect(u, v)) &&
		(!online(u, v.s)) &&
		(!online(u, v.e)) &&
		(!online(v, u.e)) &&
		(!online(v, u.s)));
}

/* 射线法判断点q与多边形polygon的位置关系，要求polygon为简单多边形，顶点逆时针排列
如果点在多边形内：   返回0
如果点在多边形边上： 返回1
如果点在多边形外： 返回2
*/
int insidepolygon(int vcount, QgsPointXY Polygon[], QgsPointXY q)
{
	int c = 0, i, n;
	LINESEG l1, l2;
	bool bintersect_a, bonline1, bonline2, bonline3;
	double r1, r2;

	l1.s = q;
	l1.e = q;
	l1.e.setX(0);
	n = vcount;
	for (i = 0; i<vcount; i++)
	{
		l2.s = Polygon[i];
		l2.e = Polygon[(i + 1) % n];
		if (online(l2, q))return 1; // 如果点在边上，返回1 
		if ((bintersect_a = intersect_A(l1, l2)) || // 相交且不在端点 
			(
			(bonline1 = online(l1, Polygon[(i + 1) % n])) && // 第二个端点在射线上 
				(
				(!(bonline2 = online(l1, Polygon[(i + 2) % n]))) && /* 前一个端点和后一个
																	端点在射线两侧 */
					((r1 = multiply(Polygon[i], Polygon[(i + 1) % n], l1.s)*multiply(Polygon[(i + 1) % n], Polygon[(i + 2) % n], l1.s))>0) ||
					(bonline3 = online(l1, Polygon[(i + 2) % n])) &&     /* 下一条边是水平线，
																		 前一个端点和后一个端点在射线两侧  */
					((r2 = multiply(Polygon[i], Polygon[(i + 2) % n], l1.s)*multiply(Polygon[(i + 2) % n],
						Polygon[(i + 3) % n], l1.s))>0)
					)
				)
			) c++;
	}
	if (c % 2 == 1)
		return 0;
	else
		return 2;
}


// 判定射线与边（线段）是否有交点。-1为没有交点，1为相交
// 2为点在线段上，3为点与线段顶点重合。
int judgeCross(QgsPointXY pnt1, QgsPointXY pnt2, QgsPointXY pnt)
{
	int ret = -1;

	if ((pnt.x() == pnt1.x()) && (pnt.y() == pnt1.y()))  // 与顶点pnt1重合；
		ret = 3;
	else if ((pnt.x() == pnt2.x() && (pnt.y() == pnt2.y())))  // 与顶点pnt2重合；
		ret = 3;

	else if ((pnt1.x() - pnt.x()) * (pnt2.x()- pnt.x()) < 0)
	{
		float z;                       // 用于定义直线交点的y值；
		z = (float)(pnt2.y() - pnt1.y()) * (pnt.x() - pnt1.x()) / (pnt2.x() - pnt1.x()) + pnt1.y();
		if (z>pnt.y()) //有交点
			ret = 1;
		else if (z == pnt.y())
			ret = 2;//点在线段上
	}

	//对于交点在线段端点，
	//可以左端点认为相交，右端点认为不相交。
	else if ((pnt1.x() - pnt.x()) * (pnt2.x() - pnt.x()) == 0)
	{
		QgsPointXY temp = (pnt1.x() < pnt2.x()) ? pnt1 : pnt2;
		if ((pnt.x() == temp.x()) && (pnt.y() < temp.y()))
			ret = 1;
	}
	return  ret;
}

// 判断点是否在多边形内
bool inPoly(QgsPointXY *pnts, int number, QgsPointXY pnt)
{
	bool beInPoly = false;
	int crossCount = 0;	   // 定义射线与边交点个数，奇数则在多边性内；
	QgsPointXY pntOld, pntNew;
	pntOld = pnts[number - 1];

	// 和所有的边都求交点，获得交点个数
	for (int i = 0; i<number; i++)
	{
		pntNew = pnts[i];
		int becross = judgeCross(pntOld, pntNew, pnt);
		if (becross == 1)
			crossCount++;   //有交点，交点个数加一；
		else if (becross == 2)
			return true;     // 点落在线段上，直接判定为在多边形内；
		else if (becross == 3)
			return false;    // 与端点重合，直接判定落于多边形外；
		pntOld = pntNew;
	}
	if (crossCount % 2 == 1)
		beInPoly = true;
	return beInPoly;
}

//判断关系，如果相交返回0，
//如果本多边形包含out_polygon返回1，
//如果out_polygon包含本多边形返回2
//如果本多边形和out_polygon没有任何关系，返回3
int CGeoPolygon::PolygonInPolygon(CGeoPolygon *out_polygon)
{
	int i;
	QVector<CGeoPoint>::iterator d_iter = out_polygon->m_polyon.begin();
	for (; d_iter != out_polygon->m_polyon.end(); d_iter++)
	{
		int point_count = d_iter->m_points.size();
		if (point_count < 1)
			continue;

		for (i = 1; i<point_count; i++)
		{
			//判断边是否和多边形相交
			if (LineIntersectPolygon(d_iter->m_points[i - 1], d_iter->m_points[i]))
				return 0;//有一条边相交了
		}
		//判断首尾的线段
		if (LineIntersectPolygon(d_iter->m_points[point_count-1], d_iter->m_points[0]))
			return 0;//有一条边相交了

		//如果本多边形包含out_polygon返回1
		for (i = 0; i<point_count; i++)
		{
			if (!InPolygon(d_iter->m_points[i].x(), d_iter->m_points[i].y()))
				break;//计算数量
		}

		if (i == point_count)
			return 1;//本多边形包含out_polygon返回1，
	}

	//还需要进一步判断
	QVector<CGeoPoint>::iterator iter = m_polyon.begin();
	for (; iter != m_polyon.end(); iter++)
	{
		int point_count = iter->m_points.size();
		if (point_count < 1)
			continue;

		for (i = 0; i<point_count; i++)
		{
			if (!out_polygon->InPolygon(iter->m_points[i].x(), iter->m_points[i].y()))
				break;//计算数量
		}
		if (i == point_count)
			return 2;//out_polygon包含本多边形返回2，
	}
	return 3;
}

bool CGeoPolygon::PolylineInPolygon(CGeoLine *line)
{
	QVector<CGeoPoint>::iterator d_iter = line->m_line.begin();
	for (; d_iter != line->m_line.end(); d_iter++)
	{
		int point_count = d_iter->m_points.size();
		for (int i = 0; i<point_count; i++)
		{
			//判断边是否和多边形相交
			if (i>0)
			{
				if (LineIntersectPolygon(d_iter->m_points[i - 1], d_iter->m_points[i]))
					return true;
			}

			if (InPolygon(d_iter->m_points[i].x(), d_iter->m_points[i].y()))
				return true;
		}
	}
	return false;
}

//线段是否相交或者线段是否在面里面
bool CGeoPolygon::LineIntersectPolygon(QgsPointXY s, QgsPointXY e)
{
	QVector<CGeoPoint>::iterator d_iter = m_polyon.begin();
	LINESEG l1, l2;
	l1.s = s;
	l1.e = e;
	for (; d_iter != m_polyon.end(); d_iter++)
	{
		int point_count = d_iter->m_points.size();
		if (point_count < 1)
			continue;

		for (int i = 1; i<point_count; i++)
		{
			//判断边是否和多边形相交
			l2.s = d_iter->m_points[i - 1];
			l2.e = d_iter->m_points[i];
			if (intersect(l1, l2))
				return true;
		}

		//判断首尾的线段
		l2.s = d_iter->m_points[point_count - 1];
		l2.e = d_iter->m_points[0];
		if (intersect(l1, l2))
			return true;
	}
	return false;
}

bool CGeoPolygon::InPolygon(double x, double y)
{//判断点是否在多边行区域内
	QgsRectangle *bound = GetBound();
	if (x<bound->xMinimum() || x>bound->xMaximum() || y<bound->yMinimum() || y>bound->yMaximum())
		return false;

	QgsPointXY pt(x,y);
	int count = m_polyon.size();
	if (count<1)
		return false;


	if (inPoly(&m_polyon[0].m_points[0], m_polyon[0].m_points.size(), pt))
	{//在该多边形区域内，还需要判断是带洞的情况
		bool flag = true;
		for (int i = 1; i<count; i++)
		{
			if (inPoly(&m_polyon[i].m_points[0], m_polyon[i].m_points.size(), pt))
			{
				flag = false;
				break;//表示是在洞里
			}
		}

		if (flag)
		{
			return true;
		}
	}

	return false;
}

