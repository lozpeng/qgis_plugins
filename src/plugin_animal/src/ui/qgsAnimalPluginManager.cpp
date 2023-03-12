#include "qgsAnimalPluginManager.h"

#include "base/QgsBaseActionFilter.h"
#include "sampling/qgsAnimalSamplGridAction.h"
#include  <qgsapplication.h>
#include <qgsmapcanvas.h>

qgsAnimalPluginManager::qgsAnimalPluginManager(QgisInterface* qgsInterface) 
		:qgsRibbonPluginUIBase(qgsInterface),
		mAnimalPage(nullptr)
{

}

qgsAnimalPluginManager::~qgsAnimalPluginManager()
{

}
void qgsAnimalPluginManager::initUi()
{
	if (mAnimalPage)return;

	setDefaultPages();
	mAnimalPage = this->getRibbonPage("mAnimalSamplingPage", true);
	if (!mAnimalPage)return;

	mAnimalPage->setTitle(QString::fromLocal8Bit("1.监测样线设计"));
	mAnimalPage->setContextColor(ContextColorOrange);
	//！将页面放到第一个去
	this->movePageTo(mAnimalPage, 0);

	//!.根据选中的多边形进行系统抽样
	//首先要转换坐标系为投影坐标系
	qgsAnimalSamplGridAction* act = new qgsAnimalSamplGridAction(this->mQgsInterface);
	RibbonGroup* rb = mAnimalPage->addGroup(QString::fromLocal8Bit("抽样"));
	rb->addAction(act->getAction());

}

void qgsAnimalPluginManager::setDefaultPages()
{
	RibbonPage* tmpPage = this->getDigitizePage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("2.矢量数据编辑"));
		tmpPage->setContextColor(ContextColorGreen);
	}

	tmpPage = this->getAttributesPage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("3.属性数据编辑"));
		tmpPage->setContextColor(ContextColorCyan);
	}

	tmpPage = this->getRasterLayerPage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("4.栅格数据"));
		tmpPage->setContextColor(ContextColorBlue);
	}

	tmpPage = this->getLabelingPage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("5.地图注记"));
		tmpPage->setContextColor(ContextColorPurple);
	}
	tmpPage = this->getDatabasePage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("7.数据库管理"));
		tmpPage->setContextColor(ContextColorNone);
	}
	tmpPage = this->getAnaylysisPage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("6.数据分析"));
		tmpPage->setContextColor(ContextColorYellow);
	}
	tmpPage = this->getPluginsPage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("9.功能扩展..."));
		tmpPage->setContextColor(ContextColorRed);
	}
	tmpPage = this->getWebPage();  //在线数据
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("8.在线数据"));
		tmpPage->setContextColor(ContextColorRed);
		tmpPage->setVisible(false);
	}
}
void qgsAnimalPluginManager::load() {
	initUi();
}
void qgsAnimalPluginManager::unload() {
	if (mAnimalPage)
	{
		this->removeRibbonPage(mAnimalPage);
		mAnimalPage->deleteLater();
		mAnimalPage = nullptr;
	}
}
