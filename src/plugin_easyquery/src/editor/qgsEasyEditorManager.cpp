#include "qgsEasyEditorManager.h"

#include <qgisinterface.h>
#include "add/qgsGeometryAddTool.h"

qgsEasyEditorManager::qgsEasyEditorManager(QgisInterface* qgsInterface, QObject* parent)
	: QObject(parent)
#ifdef WITH_QTITAN
	,qgsRibbonPluginUIBase(qgsInterface)
#endif
	, mGeometryAddTool(nullptr)
{}

qgsEasyEditorManager::~qgsEasyEditorManager()
{}
void qgsEasyEditorManager::loadUI() {
	if (!this->mQgsInterface)return;

	if (!mGeometryAddTool)
	{
		//mGeometryAddTool = new 
		QAction* mActionAddFeature = new  QAction(QIcon(":/images/themes/default/mActionAddPolygon.svg"), tr("SilenAddFeature"), this);
		mActionAddFeature->setObjectName("mActionAddFeature");
		mActionAddFeature->setData(QVariant(1));

		QObject::connect(mActionAddFeature, &QAction::triggered, [=]() {
			this->mQgsInterface->mapCanvas()->setMapTool(mGeometryAddTool);
		});
		mQgsInterface->addVectorToolBarIcon(mActionAddFeature);
		if (!mGeometryAddTool)
		{
			mGeometryAddTool = new qgsGeometryAddTool(mQgsInterface->mapCanvas());
			mGeometryAddTool->setAction(mActionAddFeature);
			mActionAddFeature->setParent(mGeometryAddTool);
		}
	}
	//



	

}
void qgsEasyEditorManager::unLoad() {
	if (!this->mQgsInterface)return;
	if (mGeometryAddTool)
		delete mGeometryAddTool;
	mGeometryAddTool = 0;
}
