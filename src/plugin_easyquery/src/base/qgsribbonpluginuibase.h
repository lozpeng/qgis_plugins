#ifndef QGS_RIB_PLUGIN_UI_BASE_H_
#define QGS_RIB_PLUGIN_UI_BASE_H_
#include <QObject>
#include <qgisinterface.h>

#ifndef _CNSTR
#define _CNSTR(a) QString::fromlocale8Bit(a)
#endif

class QDockWidget;
class QToolBar;

#ifdef WITH_QTITAN
#include <qtrib\QtitanRibbon.h>
//qgsRibbonPluginUI�����࣬��Ҫ���ڻ�ȡ����QGSĬ�ϵĳ����RibbonBar�ϵĵ�
//��ʼҳ�棬��Pages
class qgsRibbonPluginUIBase : public QObject
{
	Q_OBJECT

public:
	qgsRibbonPluginUIBase(QgisInterface* qgsInterface);
	~qgsRibbonPluginUIBase();
	QDockWidget* getLayerManager();
	QToolBar* getMainToolbar();
protected:
	//����������Ѱ��һ�������û�������bAddNew������һ��������ӵ�������
	RibbonPage* getRibbonPage(QString pageName, bool bAddNew = false);
	//�Ƴ�һ�����������ϵ�ҳǩ
	bool removeRibbonPage(RibbonPage* ribPage);
	/// <summary>
	/// ����UI������Ϣ
	/// </summary>
	void loadUIConfig();
	//
	RibbonPage* getDigitizePage();
	RibbonPage* getAttributesPage();
	RibbonPage* getRasterLayerPage();
	RibbonPage* getDatabasePage();
	RibbonPage* getLabelingPage();
	RibbonPage* getAnaylysisPage();
	RibbonPage* getWebPage();
	RibbonPage* getPluginsPage();
	QgisInterface* mQgsInterface;
	RibbonBar* mRibbonBar; //��������
	QMap<QString, RibbonPage*> mRibbonPages;
	QMap<QString, QDockWidget*> mDockWidgets;
private:
	void init();
};
#endif
#endif
