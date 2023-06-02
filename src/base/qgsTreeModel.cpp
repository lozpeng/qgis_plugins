#include "qgsTreeModel.h"

#include "qgsTreeItem.h"

/// <summary>
/// 
/// </summary>
/// <param name="headers"></param>
/// <param name="data"></param>
/// <param name="parent"></param>
qgsTreeModel::qgsTreeModel(const QStringList& headers, const QVector<QList<QVariant>>& data, QObject* parent):
	QAbstractItemModel(parent)
{
	QVector<QVariant> rootData;
	for (const QString& header : headers)
		rootData << header;

	rootItem = new qgsTreeItem(rootData);
	setupModelData(data, rootItem);
}
qgsTreeModel::~qgsTreeModel()
{
	delete rootItem;
}
/// <summary>
/// 如果超出范围则返回跟节点
/// </summary>
/// <param name="index"></param>
/// <returns></returns>
qgsTreeItem* qgsTreeModel::getItem(const QModelIndex& index)const
{
	if (index.isValid())
	{
		qgsTreeItem* item = static_cast<qgsTreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return rootItem;
}
int qgsTreeModel::rowCount(const QModelIndex& parent = QModelIndex()) const
{
	const qgsTreeItem* parentItem = getItem(parent);
	return parentItem ? parentItem->childCount() : 0;
}
QVariant qgsTreeModel::data(const QModelIndex& index, int role)const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return QVariant();

	qgsTreeItem* item = getItem(index);

	return item->data(index.column());
}

QVariant qgsTreeModel::headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

QModelIndex qgsTreeModel::index(int row, int column, const QModelIndex& parent = QModelIndex())const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();
	//! [5]

	//! [6]
	qgsTreeItem* parentItem = getItem(parent);
	if (!parentItem)
		return QModelIndex();

	qgsTreeItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	return QModelIndex();
}

QModelIndex qgsTreeModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex();

	qgsTreeItem* childItem = getItem(index);
	qgsTreeItem* parentItem = childItem ? childItem->parent() : nullptr;

	if (parentItem == rootItem || !parentItem)
		return QModelIndex();

	return createIndex(parentItem->itemIndex(), 0, parentItem);
}



int qgsTreeModel::columnCount(const QModelIndex& parent = QModelIndex()) const
{
	Q_UNUSED(parent);
	return rootItem->columnCount();
}


Qt::ItemFlags qgsTreeModel::flags(const QModelIndex& index) const {
	if (!index.isValid())
		return Qt::NoItemFlags;

	return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}
bool qgsTreeModel::removeColumns(int position, int columns, const QModelIndex& parent) {
	beginRemoveColumns(parent, position, position + columns - 1);
	const bool success = rootItem->removeColumns(position, columns);
	endRemoveColumns();

	if (rootItem->columnCount() == 0)
		removeRows(0, rowCount());

	return success;
}
bool qgsTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) 
{
	if (role != Qt::EditRole)
		return false;

	qgsTreeItem* item = getItem(index);
	bool result = item->setData(index.column(), value);

	if (result)
		emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });

	return result;
}

bool qgsTreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role) {
	if (role != Qt::EditRole || orientation != Qt::Horizontal)
		return false;

	const bool result = rootItem->setData(section, value);

	if (result)
		emit headerDataChanged(orientation, section, section);

	return result;
}

bool qgsTreeModel::insertColumns(int position, int columns, const QModelIndex& parent)
{
	beginInsertColumns(parent, position, position + columns - 1);
	const bool success = rootItem->insertColumns(position, columns);
	endInsertColumns();
	return success;
}



bool qgsTreeModel::insertRows(int position, int rows, const QModelIndex& parent)
{
	qgsTreeItem* parentItem = getItem(parent);
	if (!parentItem)
		return false;

	beginInsertRows(parent, position, position + rows - 1);
	const bool success = parentItem->insertChildren(position,
		rows,
		rootItem->columnCount());
	endInsertRows();

	return success;
}

bool qgsTreeModel::removeRows(int position, int rows, const QModelIndex& parent)
{
	qgsTreeItem* parentItem = getItem(parent);
	if (!parentItem)
		return false;

	beginRemoveRows(parent, position, position + rows - 1);
	const bool success = parentItem->removeChildren(position, rows);
	endRemoveRows();

	return success;
}
/// <summary>
/// 添加数据
/// </summary>
/// <param name="data"></param>
/// <param name="parent"></param>
void qgsTreeModel::setupModelData(const QVector<QList<QVariant>>& data, qgsTreeItem* parent)
{
	if (data.size() <= 0)return;
	qgsTreeItem* currentParent = parent;
	if (currentParent == 0)
		currentParent = rootItem; //如果指定的父节点为nullptr则将数据添加至根节点下

	int number = 0;
	//
	while (number < data.count())
	{
		QList<QVariant> columnData = data[number];
		//添加一个新的节点
		currentParent->insertChildren(currentParent->childCount(), 1, rootItem->columnCount());
		for (int column = 0; column < columnData.size(); ++column)
			currentParent->child(parent->childCount() - 1)->setData(column, columnData[column]);
		number++;
	}
}
/// <summary>
/// 
/// </summary>
/// <param name="lines">数据列表，每一项</param>
/// <param name="parent"></param>
//void qgsTreeModel::setupModelData(const QStringList& lines, qgsTreeItem* parent)
//{
//	QList<qgsTreeItem*> parents;
//	QList<int> indentations;
//	parents << parent;
//	indentations << 0;
//	int number = 0;
//
//	while (number < data.count()) {
//		int position = 0;
//		while (position < lines[number].length()) {
//			if (lines[number].at(position) != ' ')
//				break;
//			++position;
//		}
//
//		const QString lineData = lines[number].mid(position).trimmed();
//
//		if (!lineData.isEmpty()) {
//			// Read the column data from the rest of the line.
//			const QStringList columnStrings =
//				lineData.split(QLatin1Char('\t'), Qt::SkipEmptyParts);
//			QList<QVariant> columnData;
//			columnData.reserve(columnStrings.size());
//			for (const QString& columnString : columnStrings)
//				columnData << columnString;
//
//			if (position > indentations.last()) {
//				// The last child of the current parent is now the new parent
//				// unless the current parent has no children.
//
//				if (parents.last()->childCount() > 0) {
//					parents << parents.last()->child(parents.last()->childCount() - 1);
//					indentations << position;
//				}
//			}
//			else {
//				while (position < indentations.last() && parents.count() > 0) {
//					parents.pop_back();
//					indentations.pop_back();
//				}
//			}
//
//			// Append a new item to the current parent's list of children.
//			qgsTreeItem* parent = parents.last();
//			parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
//			for (int column = 0; column < columnData.size(); ++column)
//				parent->child(parent->childCount() - 1)->setData(column, columnData[column]);
//		}
//		++number;
//	}
//}

