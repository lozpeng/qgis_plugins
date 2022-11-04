#include "QManualRestEvent.h"

#include <QMutexLocker>
#include <QVector>

// ���캯��
QManualRestEvent::QManualRestEvent(bool initialState)
{   // ��ʼ����ֹ״̬���
	open = initialState;
}
// ��ֹ��ǰ�̣߳�ֱ���յ��źŻ��߳���ָ����ʱ��
bool QManualRestEvent::WaitOne(unsigned long time)
{
	QMutexLocker locker(&mutex);
	if (!open)monitor.wait(&mutex, time); // �ȴ��ź�
	return open;
}

// ���¼�״̬����Ϊ��ֹ״̬������һ�������ȴ��̼߳���
void QManualRestEvent::Set()
{
	QMutexLocker locker(&mutex);
	open = true;
	monitor.wakeAll();
}

// ���¼�״̬����Ϊ����ֹ״̬�������߳���ֹ
void QManualRestEvent::Reset()
{
	open = false;
}
//�ȴ�һ���Ѿ���������߳�
int QManualRestEvent::WaitAny(QVector<QManualRestEvent*> events)
{
	int size = events.size();
	for (int i = 0; i < size; i++)
	{
		QManualRestEvent* e= events.at(i);
		if (e->open)
			return i;
	}
	return -1;
}

QManualRestEvent::~QManualRestEvent()
{
}
