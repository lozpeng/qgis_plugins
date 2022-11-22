#ifndef QGS_BASE_ACTION_FILTER_H_
#define QGS_BASE_ACTION_FILTER_H_
//Qt
#include <QObject>
#include <QString>
#include <QIcon>
#include <QWidget>
//工具描述
struct QgsBaseActionDescription{
	QString m_action_name; //操作的名称
	QString m_entry_name;   //
	QString m_status_tip;   //状态栏提示信息
	QIcon m_icon;			//图标
	QString m_friend_name;   //用户友好名称

	QgsBaseActionDescription()
		: m_action_name("qgs_vector_ana")
		, m_entry_name("action")
		, m_status_tip(QString::fromLocal8Bit("矢量数据处理"))
		, m_icon(QIcon(QString::fromUtf8(":/easy_query/images/image_search-24px.svg")))
	{}

	QgsBaseActionDescription(QString actionName, QString entryName, QString statusTip, QString icon)
		: m_action_name(actionName)
		, m_entry_name(entryName)
		, m_status_tip(statusTip)
		, m_icon(QIcon(icon))
	{}
};
class QgisInterface;
class QAction;

class QgsBaseActionFilter : public QObject {
	Q_OBJECT

public:
	QgsBaseActionFilter(QgsBaseActionDescription desc = QgsBaseActionDescription(), QgisInterface* qgsInterface=0);
	~QgsBaseActionFilter();

	/** \brief Get the action associated with the button
	* used in menu and toolbar creation
	*/
	QAction* getAction();

	/**
	* \brief get the widget 
	*/
	QWidget* getWidget();

	//! 设置工具是否可用
	void setEnabled(bool enabled);

	//! Returns the error message corresponding to a given error code
	/** Each filter have a set of possible error message to be used given bt getFilterErrorMessage()
	Baseclass implementation provides some generic messages.
	\note These messages can be replaced by re-implementing this method and handling the same
	codes BEFORE calling the baseclass method
	**/
	virtual QString getErrorMessage(int errorCode);

	//! Returns the status tip
	/** Status tip is visualized in status bar when button is hovered.
	used in QAction creation
	**/
	QString getStatusTip() const;
	//! Returns the icon associated with the button
	QIcon getIcon() const;

	//! Returns the name of the filter
	QString getActionName() const;

	//! Sets whether to show a progressbar while computing or not
	void setShowProgressBar(bool status) { m_show_progress = status; }

	//! Performs the actual filter job
	/** This method MUST be re-implemented by derived filter
	\return 1 if successful (error code otherwise)
	**/
	virtual int compute() = 0;
	//! Returns the entry name
	/** Entry name is used when creating the corresponding QAction by initAction
	**/
	QString getEntryName() const;
	//! Sets associated CC application interface (to access DB)
	void setQgsInterface(QgisInterface* qgsInterface);

	//! Returns associated CC application interface for accessing to some of mainWindow methods
	QgisInterface * getQgsInterface() { return m_qgsInterface; }

	//！显示处理消息
	void showMessage(QString msg);
	//!
	void setOptData(int data);// { this->mData = data; }
protected:
	//! Opens the input dialog window. Where the user can supply parameters for the computation
	/** Automatically called by performAction.
	Does nothing by default. Must be overridden if a dialog
	must be displayed.
	\return 1 if dialog has been successfully executed, 0 if canceled, negative error code otherwise
	**/
	virtual int openInputDialog() { return 1; }

	//! Opens the output dialog window. To be used when the computations have output to be shown in a dedicated dialog (as plots, histograms, etc)
	/** Automatically called by performAction.
	Does nothing by default. Must be overridden if a output dialog
	must be displayed.
	\return 1 if dialog has been successfully executed, 0 if canceled, negative error code otherwise
	**/
	virtual int openOutputDialog() { return 1; }

	//! Collects parameters from the filter dialog (if openDialog is successful)
	/** Automatically called by performAction.
	Does nothing by default. Must be overridden if necessary.
	**/
	virtual void getParametersFromDialog() {}

	//! Checks that the parameters retrieved by getParametersFromDialog are valid
	/** Automatically called by performAction.
	Does nothing by default. Must be overridden if necessary.
	\return 1 if parameters are valid (error code otherwise)
	*/
	virtual int checkParameters() { return 1; }
	//! Emits the error corresponding to a given error code (see newErrorMessage)
	/** Error messages are retrieved from getErrorMessage() and getFilterErrorMessage()
	\param errCode Error code (identifies a given error message)
	**/
	void throwError(int errCode);

	//! Starts computation
	/** Automatically called by performAction.
	By default, baseclass method simply calls compute
	Can be overridden if needed (e.g. a pre-processing step before compute())
	\return 1 if whole process is successful (error code otherwise)
	**/
	virtual int start();

	//! Initializes the corresponding action
	/** Action can be retrieved with getAction.
	**/
	virtual void initAction();
protected slots :
		//! Returns is called when the dialog window is accepted.
		/** it can be overridden but normally should not be necessary
		the parameters will be retrieved from the dialog
		via the getParametersFromDialog() method
		this always need to be overridden.
		**/
		//DGM: useless as dialogs are always modal
		//virtual int dialogAccepted();

		//! Called when action is triggered
		/** \note performAction calls start() in base class
		**/
		int performAction();
protected:
	//! The filter action (created by initAction)
	QAction* m_action;
	//! 该
	QWidget* mWidget;
	//! Filter information
	/** Contains all informations about the given filter, as name etc..
	Passed to the BaseFilter class constructor.
	**/
	QgsBaseActionDescription m_desc;

	//! Associated application interface
	QgisInterface* m_qgsInterface;
	//! Do we want to show a progress bar when the filter works?
	bool m_show_progress;
	int mData = 0;
signals:
	//! Signal emitted when a new error message is produced
	void newErrorMessage(QString);
};
#endif
