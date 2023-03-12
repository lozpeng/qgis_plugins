#include "qgs_plugin_imp.h"

#include "ui/qgsAnimalPluginManager.h"

qgs_plugin_imp::qgs_plugin_imp(QgisInterface* theInterface) :
	mQGisIface(theInterface), mUIManager(nullptr)
{
	/**
	QTranslator*  translator = new QTranslator();
	QString my_qm_dir = QCoreApplication::applicationDirPath();
	QDir dir(my_qm_dir);
	if (dir.cdUp())
	my_qm_dir = dir.absolutePath().append("/i18n/");

	translator->load("silen_data_plugin_zh.qm", my_qm_dir);   //cn是翻译好的qm文件
	QCoreApplication::installTranslator(translator);
	**/
}

qgs_plugin_imp::~qgs_plugin_imp()
{
	if (mUIManager)
	{
		mUIManager->unload();
		delete mUIManager;
		mUIManager = nullptr;
	
	}
}
void qgs_plugin_imp::initGui() {

	if (!mUIManager)
		mUIManager = new qgsAnimalPluginManager(mQGisIface);

	mUIManager->load();
}
//! unload the plugin
void qgs_plugin_imp::unload() {
	if (mUIManager)
		mUIManager->unload();
}

void qgs_plugin_imp::help() {

}