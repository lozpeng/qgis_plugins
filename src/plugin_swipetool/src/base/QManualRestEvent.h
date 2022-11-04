#ifndef Q_MANUAL_REST_EVNT_H_
#define Q_MANUAL_REST_EVNT_H_

#include <QMutex>
#include <QWaitCondition>

class QManualRestEvent
{
public:

	~QManualRestEvent();
    /*
     * ���캯�������ó�ʼ״̬
     */
	QManualRestEvent(bool initialState);
 
    /*
     * ���ܣ���ֹ��ǰ�̣߳�ֱ���յ��źŻ��߳���ָ����ʱ��
     */
    bool WaitOne(unsigned long time = ULONG_MAX);
    /*
     * ���ܣ����¼�״̬����Ϊ��ֹ״̬������һ�������ȴ��̼߳���
     */
    void Set();
 
    /*
     * ���ܣ����¼�״̬����Ϊ����ֹ״̬�������߳���ֹ
     */
    void Reset();

	static int WaitAny(QVector<QManualRestEvent*> events);
private:
    /*
     * ״̬��ǣ�true��ʾ��ֹ״̬��false��ʾ����ֹ״̬
     */
    volatile bool open; // ע�⣺��ֹ��������������Ż�
 
    QMutex mutex;   // ������
    QWaitCondition monitor; // �����ȴ��ź���
};
/*
template <class T>
int getArrayLen(T& array)
{
	return (sizeof(array) / sizeof(array[0]));
}
*/
#endif

