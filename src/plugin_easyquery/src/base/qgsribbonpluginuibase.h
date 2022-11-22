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
//qgsRibbonPluginUI基础类，主要用于获取现有QGS默认的程序的RibbonBar上的的
//初始页面，的Pages
class qgsRibbonPluginUIBase : public QObject
{
	Q_OBJECT

public:
	qgsRibbonPluginUIBase(QgisInterface* qgsInterface);
	~qgsRibbonPluginUIBase();
	QDockWidget* getLayerManager();
	QToolBar* getMainToolbar();
protected:
	//在主界面中寻找一个，如果没有则根据bAddNew来创建一个，并添加到界面上
	RibbonPage* getRibbonPage(QString pageName, bool bAddNew = false);
	//移除一个主工具栏上的页签
	bool removeRibbonPage(RibbonPage* ribPage);
	/// <summary>
	/// 加载UI配置信息
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
	RibbonBar* mRibbonBar; //主工具栏
	QMap<QString, RibbonPage*> mRibbonPages;
	QMap<QString, QDockWidget*> mDockWidgets;
private:
	void init();
};
#endif
#endif
