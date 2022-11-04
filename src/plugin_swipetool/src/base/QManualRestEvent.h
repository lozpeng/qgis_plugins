#ifndef Q_MANUAL_REST_EVNT_H_
#define Q_MANUAL_REST_EVNT_H_

#include <QMutex>
#include <QWaitCondition>

class QManualRestEvent
{
public:

	~QManualRestEvent();
    /*
     * 构造函数，设置初始状态
     */
	QManualRestEvent(bool initialState);
 
    /*
     * 功能：阻止当前线程，直到收到信号或者超过指定的时间
     */
    bool WaitOne(unsigned long time = ULONG_MAX);
    /*
     * 功能：将事件状态设置为终止状态，允许一个或多个等待线程继续
     */
    void Set();
 
    /*
     * 功能：将事件状态设置为非终止状态，导致线程阻止
     */
    void Reset();

	static int WaitAny(QVector<QManualRestEvent*> events);
private:
    /*
     * 状态标记，true表示终止状态，false表示非终止状态
     */
    volatile bool open; // 注意：阻止编译器对其进行优化
 
    QMutex mutex;   // 互斥量
    QWaitCondition monitor; // 条件等待信号量
};
/*
template <class T>
int getArrayLen(T& array)
{
	return (sizeof(array) / sizeof(array[0]));
}
*/
#endif

