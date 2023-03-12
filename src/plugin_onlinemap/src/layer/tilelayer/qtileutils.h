#pragma once

#include <qgspoint.h>
#include <qgsrectangle.h>

#include "qtileinfo.h"
namespace geotile
{
	//������Ƭ��������
	class qTileUtils
	{
	public:
		//���ݷֱ��ʻ�ȡ��ǰ����ļ���
		static double GetLevelByDis(double dis);
		//��ȡ�����һ��������ݷֱ���
		static long GetNearestLevelByDis(double dis);
		//���ݼ�����㵱ǰ����Ŀ��
		static double GetDisByLevel(int level);
		//��ȡ��һ����ı��
		static void GetLowIds(__int64 pid, __int64* bid);
		//
		static long GetIndex(double coord, int level, int gridSize);
		//���ݵ���γ������ͼ������������к�
		static long GetIndex(double coord, int level);
		//���ݲ�����Ͻǵ��������������
		static LONGLONG GetTileId(double coordx, double coordy, int level);
		//�������кźͼ����м������������귶Χ
		static ENVELOPE GetRectByIndex(long xIndex, long yIndex, int level, int gridSize);
		////�������кźͼ����м��㾭γ�ȵ���������귶Χ
		static ENVELOPE GetRectByIndex(long xIndex, long yIndex, int level);
		//���ݱ�ż�����ĵ���γ�Ⱦ��ο�����
		static ENVELOPE GetGeoRectById(LONGLONG tileId);
		//�������Χ
		static ENVELOPE GetGeoRectById(LONGLONG tileId, int gridSize);
		//���ݼ����������ֵ
		static double GetCoord(long indx, int level);
		//�������кż����������
		static double GetCoord(long indx, int level, int gridSize);
		//�������кż����������
		static void GetBlockRect(long x, long y, int level, ENVELOPE& extent);
		//�������кż�����Ƭ��Χ
		static void GetBlockRect(long x, long y, int level, QgsRectangle& rect);
		//����һ�еĿ��
		static double GetLevelWidth(int level);
	};
}
