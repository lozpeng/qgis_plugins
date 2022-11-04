#include "vqgsmapswiper.h"

#include <QAction>
#include <QComboBox>

#include <qgsmapcanvas.h>
#include <qgsproject.h>
#include <qgslayertree.h>
#include <qgslayertreenode.h>
#include <qgsmaptoolpan.h>

vQgsMapSwiper::vQgsMapSwiper(QgisInterface * qgsInterface)
	: QObject(), 
	mQgsInterface(qgsInterface),
	mAction(0), mXAction(0), mXSwiperTool(0)
{
	mPrevTool = mQgsInterface->mapCanvas()->mapTool();
	connect(mQgsInterface->mapCanvas(), &QgsMapCanvas::mapToolSet, this, &vQgsMapSwiper::on_mapToolSet);
}

vQgsMapSwiper::~vQgsMapSwiper()
{
	disconnect(mQgsInterface->mapCanvas(), &QgsMapCanvas::mapToolSet, this, &vQgsMapSwiper::on_mapToolSet);
	if (mAction)
		delete mAction;
	mAction = nullptr;

	if (mSwiperTool)
		delete mSwiperTool;
	mSwiperTool = nullptr;

	if (mXAction)
		delete mXAction;
	mXAction = nullptr;

	if (mXSwiperTool)
		delete mXSwiperTool;
	mXSwiperTool = nullptr;

}


void vQgsMapSwiper::init()
{
	if (!mAction)
	{
		mAction = new QAction(QIcon(":/qgsml/images/icon.png"),
				QString::fromLocal8Bit("¾íÁ±"), this);

		mAction->setCheckable(true);
		connect(mAction, &QAction::triggered, this, &vQgsMapSwiper::on_ation_triggered);
		mQgsInterface->addRasterToolBarIcon(mAction);
		int height = mQgsInterface->iconSize().height() + 8;
		mSwiperTool = new vQgsLayerSwiperTool(mQgsInterface->mapCanvas(), height);
	}

	if (!mXAction)
	{
		mXAction = new QAction(QIcon(":/qgsml/images/pan.png"), 
								QString::fromLocal8Bit("·Å´ó¾µ"), this);

		mXAction->setCheckable(true);
		connect(mXAction, &QAction::triggered, this, &vQgsMapSwiper::on_xation_triggered);
		mQgsInterface->addRasterToolBarIcon(mXAction);
		int height = mQgsInterface->iconSize().height() + 8;
		mXSwiperTool = new vQgsXRayMagnifyTool(mQgsInterface->mapCanvas(), height);
	}
}
void vQgsMapSwiper::on_ation_triggered(bool checked)
{
	if (checked)
	{
		mPrevTool = mQgsInterface->mapCanvas()->mapTool();
		mQgsInterface->mapCanvas()->setMapTool(mSwiperTool);
		if (mXAction && mXAction->isChecked())
			mXAction->setChecked(false);
	}
	else {
		if(!mPrevTool)
			mQgsInterface->mapCanvas()->setMapTool(new QgsMapToolPan(mQgsInterface->mapCanvas()));
		else
			mQgsInterface->mapCanvas()->setMapTool(mPrevTool);
	}
	
}
void vQgsMapSwiper::on_xation_triggered(bool checked)
{
	if (checked)
	{
		mPrevTool = mQgsInterface->mapCanvas()->mapTool();
		mQgsInterface->mapCanvas()->setMapTool(mXSwiperTool);
		if (mAction && mAction->isChecked())
			mAction->setChecked(false);
	}
	else {
		if (!mPrevTool)
			mQgsInterface->mapCanvas()->setMapTool(new QgsMapToolPan(mQgsInterface->mapCanvas()));
		else
			mQgsInterface->mapCanvas()->setMapTool(mPrevTool);
	}
}

void vQgsMapSwiper::on_mapToolSet(QgsMapTool *newTool, QgsMapTool *oldTool)
{
	if (newTool != mXSwiperTool && mXAction)
		mXAction->setChecked(false);
	if (newTool != mSwiperTool && mAction)
		mAction->setChecked(false);
		
}
void vQgsMapSwiper::unload()
{
	/*
	if (mPrevTool && mPrevTool != mSwiperTool  &&mPrevTool!= mXSwiperTool)
		mQgsInterface->mapCanvas()->setMapTool(mPrevTool);
	else mQgsInterface->mapCanvas()->setMapTool(new QgsMapToolPan(mQgsInterface->mapCanvas()));
	*/

	mQgsInterface->removeRasterToolBarIcon(mAction);
	disconnect(mAction, &QAction::triggered, this, &vQgsMapSwiper::on_ation_triggered);

	mQgsInterface->removeRasterToolBarIcon(mXAction);
	disconnect(mXAction, &QAction::triggered, this, &vQgsMapSwiper::on_xation_triggered);
}
