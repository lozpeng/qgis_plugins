#include "qgsribbonpluginuibase.h"
#include <QDockWidget>
#include <QToolBar>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>

#ifdef WITH_QTITAN
qgsRibbonPluginUIBase::qgsRibbonPluginUIBase(QgisInterface* qgsInterface)
	: QObject(qgsInterface), mRibbonBar(nullptr), mQgsInterface(qgsInterface)
{
	mRibbonPages.clear();
	init();
}

RibbonPage* qgsRibbonPluginUIBase::getDigitizePage() { return getRibbonPage("mRibbonDigitize"); }
RibbonPage* qgsRibbonPluginUIBase::getAttributesPage() { return getRibbonPage("mRibbonAttributes"); }
RibbonPage* qgsRibbonPluginUIBase::getRasterLayerPage() { return getRibbonPage("mRibbonRasterLayers"); }
RibbonPage* qgsRibbonPluginUIBase::getDatabasePage() { return getRibbonPage("mRibbonDatabase"); }
RibbonPage* qgsRibbonPluginUIBase::getLabelingPage() { return getRibbonPage("mRibbonLabeling"); }
RibbonPage* qgsRibbonPluginUIBase::getAnaylysisPage() { return getRibbonPage("mRibbonAnalysis"); }
RibbonPage* qgsRibbonPluginUIBase::getWebPage() { return getRibbonPage("mRibbonWeb"); }
RibbonPage* qgsRibbonPluginUIBase::getPluginsPage() { return getRibbonPage("mRibbonPlugins"); }

qgsRibbonPluginUIBase::~qgsRibbonPluginUIBase()
{
	mRibbonPages.clear();
}
//
void qgsRibbonPluginUIBase::init()
{
	if (!mQgsInterface)return;
	QWidget* qgsApp = mQgsInterface->mainWindow();
	if (qgsApp)
	{
		auto ribbonbars = qgsApp->findChildren<RibbonBar*>(QString(), Qt::FindDirectChildrenOnly);
		Q_FOREACH(RibbonBar * ribon, ribbonbars)
		{
			if (!ribon)continue;
			mRibbonBar = ribon;
			if (mRibbonBar)break;
		}
		if (mRibbonBar)
		{
			auto ribPages = mRibbonBar->findChildren<RibbonPage*>(QString(), Qt::FindDirectChildrenOnly);
			Q_FOREACH(RibbonPage * page, ribPages)
			{
				if (page) {
					QString name = page->objectName();
					if (!mRibbonPages.contains(name))//如果不存在
						mRibbonPages.insert(name, page);
				}
			}
		}

		auto dockWidgets = qgsApp->findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly);
		Q_FOREACH(QDockWidget * docker, dockWidgets)
		{
			if (!docker || docker->objectName().isEmpty()
				|| docker->objectName().isNull())continue;

			else mDockWidgets.insert(docker->objectName(), docker);
		}
		/*
		auto menuBars = qgsApp->findChildren<QMenuBar*>(QString(), Qt::FindDirectChildrenOnly);
		if (menuBars.count() >= 1)
		{
			Q_FOREACH(QMenuBar* menub, menuBars)
			{
				menub->setVisible(false);
			}
		}
		*/
	}
}
//
RibbonPage* qgsRibbonPluginUIBase::getRibbonPage(QString pageName, bool bAddNew)
{
	if (!mRibbonBar)return nullptr;

	if (mRibbonPages.contains(pageName))
		return mRibbonPages.value(pageName, nullptr);

	auto ribPages = mRibbonBar->findChildren<RibbonPage*>(pageName, Qt::FindDirectChildrenOnly);
	if (ribPages.count() <= 0 && bAddNew) {
		RibbonPage* tmpPage = mRibbonBar->addPage(pageName);
		if (tmpPage)
		{
			tmpPage->setContextColor(ContextColor::ContextColorCyan);
			tmpPage->setObjectName(pageName);
			tmpPage->setParent(mRibbonBar);
			mRibbonPages.insert(pageName, tmpPage);
			return tmpPage;
		}
		return 0;
	}
	RibbonPage* tpage = mRibbonBar->findChild<RibbonPage*>(pageName);
	if (tpage)
	{
		if (!mRibbonPages.contains(pageName))//如果不存在
			mRibbonPages.insert(pageName, tpage);
		return tpage;
	}
	else
	{
		if (bAddNew)
		{
			RibbonPage* tmpPage = mRibbonBar->addPage(pageName);
			if (tmpPage)
			{
				tmpPage->setContextColor(ContextColor::ContextColorCyan);
				tmpPage->setObjectName(pageName);
				mRibbonPages.insert(pageName, tmpPage);
				return tmpPage;
			}
		}
		return 0;
	}
	//RibbonPage *page = Q_CHILD(mRibbonBar, RibbonPage, pageName);
}
/// <summary>
/// 获取主工具条
/// </summary>
/// <param name="docker"></param>
/// <param name=""></param>
/// <returns></returns>
QToolBar* qgsRibbonPluginUIBase::getMainToolbar()
{
	//mMapNavToolBar
	if (!mQgsInterface)return 0;
	QWidget* qgsApp = mQgsInterface->mainWindow();
	if (qgsApp)
	{
		auto toolbars = qgsApp->findChildren<QToolBar*>("mMapNavToolBar", Qt::FindDirectChildrenOnly);
		Q_FOREACH(QToolBar * toolbar, toolbars)
		{
			if (!toolbar || toolbar->objectName().isEmpty()
				|| toolbar->objectName().isNull())continue;
			else {
				if (QString("mMapNavToolBar").compare(toolbar->objectName(), Qt::CaseInsensitive) == 0)
					return toolbar;
				else return 0;
			}
		}
		return 0;
	}
	else return nullptr;
}

/// <summary>
/// 加载用户配置的界面，主要针对Ribbon的Pages的可视性进行定义与配置
/// </summary>
void qgsRibbonPluginUIBase::loadUIConfig()
{
	if (mRibbonBar)
	{
		auto ribPages = mRibbonBar->findChildren<RibbonPage*>(QString(), Qt::FindDirectChildrenOnly);
		Q_FOREACH(RibbonPage * page, ribPages)
		{
			if (page) {
				page->setVisible(false); //先把现有的所有的Page都隐藏，使之不可见
			}
		}
		//!获取Dll的路径，然后获取
		QString my_qm_dir = QCoreApplication::applicationDirPath();// .append("/i18n/");
		QDir dir(my_qm_dir);
		if (dir.cdUp())
			my_qm_dir = dir.absolutePath().append("/ui/");
		QString initFile = QString("%1/qPluginUIConfig.ini").arg(my_qm_dir);
		QSettings settings(initFile, QSettings::IniFormat);
		//settings.status()== QSettings::Status::AccessError
		settings.beginGroup("RibbonUI"); //加载UI部分
		const QStringList childKeys = settings.childKeys();
		if (childKeys.count() >= 1)
		{
			//当配置了WebPage且可见，则使之可见
			if (childKeys.contains("WebPage") && settings.value("WebPage").toBool())
			{
				RibbonPage* page = this->getWebPage();
				if (page)page->setVisible(true);
			}
			if (childKeys.contains("Digitize") && settings.value("Digitize").toBool())
			{
				RibbonPage* page = this->getDigitizePage();
				if (page)page->setVisible(true);
			}
			if (childKeys.contains("Attribute") && settings.value("Attribute").toBool())
			{
				RibbonPage* page = this->getAttributesPage();
				if (page)page->setVisible(true);
			}
			if (childKeys.contains("Raster") && settings.value("Raster").toBool())
			{
				RibbonPage* page = this->getRasterLayerPage();
				if (page)page->setVisible(true);
			}
			if (childKeys.contains("DataBase") && settings.value("DataBase").toBool())
			{
				RibbonPage* page = this->getDatabasePage();
				if (page)page->setVisible(true);
			}
			if (childKeys.contains("Label") && settings.value("Label").toBool())
			{
				RibbonPage* page = this->getLabelingPage();
				if (page)page->setVisible(true);
			}
			if (childKeys.contains("Analysis") && settings.value("Analysis").toBool())
			{
				RibbonPage* page = this->getAnaylysisPage();
				if (page)page->setVisible(true);
			}
			if (childKeys.contains("Plugins") && settings.value("Plugins").toBool())
			{
				RibbonPage* page = this->getPluginsPage();
				if (page)page->setVisible(true);
			}
			settings.endGroup();
		}
		settings.beginGroup("Application");
		QString title = settings.value("Title", "").toString();
		if (!title.isNull() && !title.isEmpty())
		{
			QWidget* qgsApp = mQgsInterface->mainWindow();
			if (qgsApp)
				qgsApp->setWindowTitle(title);
		}
	}
}

//获取图层控制面板
QDockWidget* qgsRibbonPluginUIBase::getLayerManager()
{
	if (mDockWidgets.contains("Layers"))
		return mDockWidgets.value("Layers");
	else return nullptr;
}

bool qgsRibbonPluginUIBase::removeRibbonPage(RibbonPage* ribPage)
{
	if (ribPage && mRibbonBar)
	{
		mRibbonBar->removePage(ribPage);
		return true;
	}
	return false;
}
//！移动
void qgsRibbonPluginUIBase::movePageTo(RibbonPage* page, int pos)
{
	int indx = pos;
	if (page == nullptr)return;
	if (indx < 0)indx = 0;
	int count = this->mRibbonBar->pageCount();
	if (indx > count)
		indx = count;
	this->mRibbonBar->movePage(page, indx);
}
#endif