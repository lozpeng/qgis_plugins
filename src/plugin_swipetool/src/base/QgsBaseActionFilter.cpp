#include "QgsBaseActionFilter.h"
//Qt
#include <QAction>
#include <QFuture>
#include <QApplication>
#include <QProgressDialog>
#include <qtconcurrentrun.h>
#include <QMessageBox>
#include <QMainwindow>

//qgis
#include <qgisinterface.h>
#include "qgslogger.h"
#include "qgsmessagelog.h"
//system
#if defined(_WINDOWS)
#include "windows.h"
#else
#include <time.h>
#endif

QgsBaseActionFilter::QgsBaseActionFilter(QgsBaseActionDescription desc , 
	QgisInterface* qgsInterface)
	:m_action(0),
	m_desc(desc),	
	m_show_progress(true),
	m_qgsInterface(qgsInterface)
{
	initAction();
}


QgsBaseActionFilter::~QgsBaseActionFilter()
{
	if (m_action)
	{
		delete m_action;
		m_action = nullptr;
	}
}

void QgsBaseActionFilter::initAction()
{
	if (m_action)
		return;

	m_action = new QAction(getIcon(), getEntryName(), this);
	m_action->setStatusTip(getStatusTip());
	//connect this action
	connect(m_action, SIGNAL(triggered()), this, SLOT(performAction()));
}

void QgsBaseActionFilter::throwError(int errCode)
{
	QString errMsg = getErrorMessage(errCode);

	//DGM: libraries shouldn't issue message themselves! The information should be sent to the plugin!
	emit newErrorMessage(errMsg);
}

//响应按钮的点击事件
int QgsBaseActionFilter::performAction()
{
	//if dialog is needed open the dialog
	int dialog_result = openInputDialog();
	if (dialog_result < 1)
	{
		if (dialog_result < 0)
			throwError(dialog_result);
		else
			dialog_result = 1; //the operation is canceled by the user, no need to throw an error!
		return dialog_result;
	}

	//get the parameters from the dialog
	getParametersFromDialog();

	//are the given parameters ok?
	int par_status = checkParameters();
	if (par_status != 1)
	{
		throwError(par_status);
		return par_status;
	}

	//if so go ahead with start()
	int start_status = start();
	if (start_status != 1)
	{
		throwError(start_status);
		return start_status;
	}

	//if we have an output dialog is time to show it
	int out_dialog_result = openOutputDialog();
	if (out_dialog_result < 1)
	{
		if (out_dialog_result<0)
			throwError(out_dialog_result);
		else
			out_dialog_result = 1; //the operation is canceled by the user, no need to throw an error!
		return out_dialog_result; //maybe some filter could ask the user if he wants to ac
	}

	return 1;
}

static QgsBaseActionFilter* s_filter = 0;
static int s_computeStatus = 0;
static bool s_computing = false;
static void doCompute()
{
	if (!s_filter)
	{
		s_computeStatus = -1;
		return;
	}
	s_computeStatus = s_filter->compute();
}

int QgsBaseActionFilter::start()
{
	if (s_computing)
	{
		throwError(-32);
		return -1;
	}

	QProgressDialog progressCb(QString::fromLocal8Bit("数据处理中..."), QString(), 0, 0);

	if (m_show_progress)
	{
		progressCb.setWindowTitle(getActionName());
		progressCb.show();
		QApplication::processEvents();
	}

	s_computeStatus = -1;
	s_filter = this;
	s_computing = true;
	int progress = 0;

	QFuture<void> future = QtConcurrent::run(doCompute);
	while (!future.isFinished())
	{
#if defined(_WINDOWS)
		::Sleep(500);
#else
		usleep(500 * 1000);
#endif
		if (m_show_progress)
			progressCb.setValue(++progress);
	}

	int is_ok = s_computeStatus;
	s_filter = 0;
	s_computing = false;

	if (m_show_progress)
	{
		progressCb.close();
		QApplication::processEvents();
	}

	if (is_ok < 0)
	{
		throwError(is_ok);
		return -1;
	}

	return 1;
}

QString QgsBaseActionFilter::getActionName() const
{
	return m_desc.m_action_name;
}

QString QgsBaseActionFilter::getEntryName() const {
	return m_desc.m_entry_name;
}

QString QgsBaseActionFilter::getStatusTip() const
{
	return m_desc.m_status_tip;
}

QIcon QgsBaseActionFilter::getIcon() const
{
	return m_desc.m_icon;
}

QAction* QgsBaseActionFilter::getAction()
{
	return m_action;
}

QString QgsBaseActionFilter::getErrorMessage(int errorCode)
{
	QString errorMsg;
	switch (errorCode)
	{
		//ERRORS RELATED TO SELECTION
	case -11:
		errorMsg = QString::fromLocal8Bit("请选择一个数据！");
		break;

	case -12:
		errorMsg = QString::fromLocal8Bit("请选择一个数据！");
		break;

	case -13:
		errorMsg = QString::fromLocal8Bit("选择了错误的数据！");
		break;

		//ERRORS RELATED TO DIALOG
	case -21:
		errorMsg = QString::fromLocal8Bit("参数对话框输入错误！");
		break;

		//ERRORS RELATED TO COMPUTATION
	case -31:
		errorMsg = QString::fromLocal8Bit("计算中发生错误！");
		break;
	case -32:
		errorMsg = QString::fromLocal8Bit("线程正在运行中!");
		break;

		// DEFAULT
	default:
		errorMsg = QString(QString::fromLocal8Bit("未定义的错误: ")
			+ getActionName());
		break;
	}

	return errorMsg;
}

void QgsBaseActionFilter::setQgsInterface(QgisInterface* app) {
	m_qgsInterface = app;
	if (m_qgsInterface)
	{
		QgsMessageLog::logMessage(QString::fromLocal8Bit("插件加载成功..."), "QGS_VECTOR",
							Qgis::MessageLevel::Info);
	}
}
void QgsBaseActionFilter::showMessage(QString msg)
{
	if (m_qgsInterface)
	{
		QgsMessageLog::logMessage(msg, "QGS_VECTOR", Qgis::MessageLevel::Info);
	}
}