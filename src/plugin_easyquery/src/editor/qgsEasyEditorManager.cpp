#include "qgsEasyEditorManager.h"

qgsEasyEditorManager::qgsEasyEditorManager(QgisInterface* qgsInterface, QObject* parent)
	: QObject(parent)
#ifdef WITH_QTITAN
	,qgsRibbonPluginUIBase(qgsInterface)
#endif
{}

qgsEasyEditorManager::~qgsEasyEditorManager()
{}
void qgsEasyEditorManager::loadUI() {
	if (!this->mQgsInterface)return;


}
void qgsEasyEditorManager::unLoad() {
	if (!this->mQgsInterface)return;

}
