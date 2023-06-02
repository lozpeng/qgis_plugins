#include "qgsPluginUIManager.h"

#include "base/QgsBaseActionFilter.h"
#include  <qgsapplication.h>
#include <qgsmapcanvas.h>


//!添加自定义的按钮
#include "layer/tdt/qgsAddTDTLayerAction.h"


qgsPluginUIManager::qgsPluginUIManager(QgisInterface* qgsInterface)
		:qgsRibbonPluginUIBase(qgsInterface),
		mRibbonPage(nullptr)
{

}

qgsPluginUIManager::~qgsPluginUIManager()
{

}
void qgsPluginUIManager::initUi()
{
	if (mRibbonPage)return;

	mRibbonPage = getRasterLayerPage();
	if (!mRibbonPage)return;
	RibbonGroup* rb = mRibbonPage->addGroup(QString::fromLocal8Bit("天地图"));
	if (!rb)
		return;
	qgsAddTDTLayerAction* act = new qgsAddTDTLayerAction(this->mQgsInterface);

	rb->addAction(act->getAction());
}

void qgsPluginUIManager::load() {
	initUi();
}
void qgsPluginUIManager::unload() {
	if (mRibbonPage)
	{
		this->removeRibbonPage(mRibbonPage);
		mRibbonPage->deleteLater();
		mRibbonPage = nullptr;
	}
}
