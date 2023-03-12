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

	mAnimalPage->setTitle(QString::fromLocal8Bit("1.����������"));
	mAnimalPage->setContextColor(ContextColorOrange);
	//����ҳ��ŵ���һ��ȥ
	this->movePageTo(mAnimalPage, 0);

	//!.����ѡ�еĶ���ν���ϵͳ����
	//����Ҫת������ϵΪͶӰ����ϵ
	qgsAnimalSamplGridAction* act = new qgsAnimalSamplGridAction(this->mQgsInterface);
	RibbonGroup* rb = mAnimalPage->addGroup(QString::fromLocal8Bit("����"));
	rb->addAction(act->getAction());

}

void qgsAnimalPluginManager::setDefaultPages()
{
	RibbonPage* tmpPage = this->getDigitizePage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("2.ʸ�����ݱ༭"));
		tmpPage->setContextColor(ContextColorGreen);
	}

	tmpPage = this->getAttributesPage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("3.�������ݱ༭"));
		tmpPage->setContextColor(ContextColorCyan);
	}

	tmpPage = this->getRasterLayerPage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("4.դ������"));
		tmpPage->setContextColor(ContextColorBlue);
	}

	tmpPage = this->getLabelingPage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("5.��ͼע��"));
		tmpPage->setContextColor(ContextColorPurple);
	}
	tmpPage = this->getDatabasePage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("7.���ݿ����"));
		tmpPage->setContextColor(ContextColorNone);
	}
	tmpPage = this->getAnaylysisPage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("6.���ݷ���"));
		tmpPage->setContextColor(ContextColorYellow);
	}
	tmpPage = this->getPluginsPage();
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("9.������չ..."));
		tmpPage->setContextColor(ContextColorRed);
	}
	tmpPage = this->getWebPage();  //��������
	if (tmpPage)
	{
		tmpPage->setTitle(QString::fromLocal8Bit("8.��������"));
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
