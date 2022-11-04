#include "QManualRestEvent.h"

#include <QMutexLocker>
#include <QVector>

// 构造函数
QManualRestEvent::QManualRestEvent(bool initialState)
{   // 初始化终止状态标记
	open = initialState;
}
// 阻止当前线程，直到收到信号或者超过指定的时间
bool QManualRestEvent::WaitOne(unsigned long time)
{
	QMutexLocker locker(&mutex);
	if (!open)monitor.wait(&mutex, time); // 等待信号
	return open;
}

// 将事件状态设置为终止状态，允许一个或多个等待线程继续
void QManualRestEvent::Set()
{
	QMutexLocker locker(&mutex);
	open = true;
	monitor.wakeAll();
}

// 将事件状态设置为非终止状态，导致线程阻止
void QManualRestEvent::Reset()
{
	open = false;
}
//等待一个已经处理完的线程
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
