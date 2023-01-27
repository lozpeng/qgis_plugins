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
//!��qgsGeometry����ת��ΪCGeoPolygon����Ҫ��qgsGeometryΪ�����������
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
r=multiply(sp,ep,op),�õ�(sp-op)*(ep-op)�Ĳ��
r>0:ep��ʸ��opsp����ʱ�뷽��
r=0��opspep���㹲�ߣ�
r<0:ep��ʸ��opsp��˳ʱ�뷽��
*******************************************************************************/
double multiply(QgsPointXY sp, QgsPointXY ep, QgsPointXY op)
{
	return((sp.x() - op.x())*(ep.y() - op.y()) - (ep.x() - op.x())*(sp.y() - op.y()));
}

/* �жϵ�p�Ƿ����߶�l�ϣ�������(p���߶�l���ڵ�ֱ����)&& (��p�����߶�lΪ�Խ��ߵľ�����) */
bool online(LINESEG l, QgsPointXY p)
{
	return((multiply(l.e, p, l.s) == 0)
		&& (((p.x() - l.s.x())*(p.x() - l.e.x()) <= 0) && ((p.y() - l.s.y())*(p.y() - l.e.y()) <= 0)));
}

// ����߶�u��v�ཻ(�����ཻ�ڶ˵㴦)ʱ������true 
bool intersect(LINESEG u, LINESEG v)
{
	return((__max(u.s.x(), u.e.x()) >= __min(v.s.x(), v.e.x())) &&                     //�ų�ʵ�� 
		(__max(v.s.x(), v.e.x()) >= __min(u.s.x(), u.e.x())) &&
		(__max(u.s.y(), u.e.y()) >= __min(v.s.y(), v.e.y())) &&
		(__max(v.s.y(), v.e.y()) >= __min(u.s.y(), u.e.y())) &&
		(multiply(v.s, u.e, u.s)*multiply(u.e, v.e, u.s) >= 0) &&         //����ʵ�� 
		(multiply(u.s, v.e, v.s)*multiply(v.e, u.e, v.s) >= 0));
}

//  (�߶�u��v�ཻ)&&(���㲻��˫���Ķ˵�) ʱ����true    
bool intersect_A(LINESEG u, LINESEG v)
{
	return((intersect(u, v)) &&
		(!online(u, v.s)) &&
		(!online(u, v.e)) &&
		(!online(v, u.e)) &&
		(!online(v, u.s)));
}

/* ���߷��жϵ�q������polygon��λ�ù�ϵ��Ҫ��polygonΪ�򵥶���Σ�������ʱ������
������ڶ�����ڣ�   ����0
������ڶ���α��ϣ� ����1
������ڶ�����⣺ ����2
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
		if (online(l2, q))return 1; // ������ڱ��ϣ�����1 
		if ((bintersect_a = intersect_A(l1, l2)) || // �ཻ�Ҳ��ڶ˵� 
			(
			(bonline1 = online(l1, Polygon[(i + 1) % n])) && // �ڶ����˵��������� 
				(
				(!(bonline2 = online(l1, Polygon[(i + 2) % n]))) && /* ǰһ���˵�ͺ�һ��
																	�˵����������� */
					((r1 = multiply(Polygon[i], Polygon[(i + 1) % n], l1.s)*multiply(Polygon[(i + 1) % n], Polygon[(i + 2) % n], l1.s))>0) ||
					(bonline3 = online(l1, Polygon[(i + 2) % n])) &&     /* ��һ������ˮƽ�ߣ�
																		 ǰһ���˵�ͺ�һ���˵�����������  */
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


// �ж�������ߣ��߶Σ��Ƿ��н��㡣-1Ϊû�н��㣬1Ϊ�ཻ
// 2Ϊ�����߶��ϣ�3Ϊ�����߶ζ����غϡ�
int judgeCross(QgsPointXY pnt1, QgsPointXY pnt2, QgsPointXY pnt)
{
	int ret = -1;

	if ((pnt.x() == pnt1.x()) && (pnt.y() == pnt1.y()))  // �붥��pnt1�غϣ�
		ret = 3;
	else if ((pnt.x() == pnt2.x() && (pnt.y() == pnt2.y())))  // �붥��pnt2�غϣ�
		ret = 3;

	else if ((pnt1.x() - pnt.x()) * (pnt2.x()- pnt.x()) < 0)
	{
		float z;                       // ���ڶ���ֱ�߽����yֵ��
		z = (float)(pnt2.y() - pnt1.y()) * (pnt.x() - pnt1.x()) / (pnt2.x() - pnt1.x()) + pnt1.y();
		if (z>pnt.y()) //�н���
			ret = 1;
		else if (z == pnt.y())
			ret = 2;//�����߶���
	}

	//���ڽ������߶ζ˵㣬
	//������˵���Ϊ�ཻ���Ҷ˵���Ϊ���ཻ��
	else if ((pnt1.x() - pnt.x()) * (pnt2.x() - pnt.x()) == 0)
	{
		QgsPointXY temp = (pnt1.x() < pnt2.x()) ? pnt1 : pnt2;
		if ((pnt.x() == temp.x()) && (pnt.y() < temp.y()))
			ret = 1;
	}
	return  ret;
}

// �жϵ��Ƿ��ڶ������
bool inPoly(QgsPointXY *pnts, int number, QgsPointXY pnt)
{
	bool beInPoly = false;
	int crossCount = 0;	   // ����������߽���������������ڶ�����ڣ�
	QgsPointXY pntOld, pntNew;
	pntOld = pnts[number - 1];

	// �����еı߶��󽻵㣬��ý������
	for (int i = 0; i<number; i++)
	{
		pntNew = pnts[i];
		int becross = judgeCross(pntOld, pntNew, pnt);
		if (becross == 1)
			crossCount++;   //�н��㣬���������һ��
		else if (becross == 2)
			return true;     // �������߶��ϣ�ֱ���ж�Ϊ�ڶ�����ڣ�
		else if (becross == 3)
			return false;    // ��˵��غϣ�ֱ���ж����ڶ�����⣻
		pntOld = pntNew;
	}
	if (crossCount % 2 == 1)
		beInPoly = true;
	return beInPoly;
}

//�жϹ�ϵ������ཻ����0��
//���������ΰ���out_polygon����1��
//���out_polygon����������η���2
//���������κ�out_polygonû���κι�ϵ������3
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
			//�жϱ��Ƿ�Ͷ�����ཻ
			if (LineIntersectPolygon(d_iter->m_points[i - 1], d_iter->m_points[i]))
				return 0;//��һ�����ཻ��
		}
		//�ж���β���߶�
		if (LineIntersectPolygon(d_iter->m_points[point_count-1], d_iter->m_points[0]))
			return 0;//��һ�����ཻ��

		//���������ΰ���out_polygon����1
		for (i = 0; i<point_count; i++)
		{
			if (!InPolygon(d_iter->m_points[i].x(), d_iter->m_points[i].y()))
				break;//��������
		}

		if (i == point_count)
			return 1;//������ΰ���out_polygon����1��
	}

	//����Ҫ��һ���ж�
	QVector<CGeoPoint>::iterator iter = m_polyon.begin();
	for (; iter != m_polyon.end(); iter++)
	{
		int point_count = iter->m_points.size();
		if (point_count < 1)
			continue;

		for (i = 0; i<point_count; i++)
		{
			if (!out_polygon->InPolygon(iter->m_points[i].x(), iter->m_points[i].y()))
				break;//��������
		}
		if (i == point_count)
			return 2;//out_polygon����������η���2��
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
			//�жϱ��Ƿ�Ͷ�����ཻ
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

//�߶��Ƿ��ཻ�����߶��Ƿ���������
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
			//�жϱ��Ƿ�Ͷ�����ཻ
			l2.s = d_iter->m_points[i - 1];
			l2.e = d_iter->m_points[i];
			if (intersect(l1, l2))
				return true;
		}

		//�ж���β���߶�
		l2.s = d_iter->m_points[point_count - 1];
		l2.e = d_iter->m_points[0];
		if (intersect(l1, l2))
			return true;
	}
	return false;
}

bool CGeoPolygon::InPolygon(double x, double y)
{//�жϵ��Ƿ��ڶ����������
	QgsRectangle *bound = GetBound();
	if (x<bound->xMinimum() || x>bound->xMaximum() || y<bound->yMinimum() || y>bound->yMaximum())
		return false;

	QgsPointXY pt(x,y);
	int count = m_polyon.size();
	if (count<1)
		return false;


	if (inPoly(&m_polyon[0].m_points[0], m_polyon[0].m_points.size(), pt))
	{//�ڸö���������ڣ�����Ҫ�ж��Ǵ��������
		bool flag = true;
		for (int i = 1; i<count; i++)
		{
			if (inPoly(&m_polyon[i].m_points[0], m_polyon[i].m_points.size(), pt))
			{
				flag = false;
				break;//��ʾ���ڶ���
			}
		}

		if (flag)
		{
			return true;
		}
	}

	return false;
}

