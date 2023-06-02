#include "qgsTreeItem.h"

qgsTreeItem::qgsTreeItem(const QVector<QVariant>& data, qgsTreeItem* parent):
	itemData(data),
	parentItem(parent)
{

}
qgsTreeItem::~qgsTreeItem()
{
	qDeleteAll(childItems);
}
/// <summary>
/// 
/// </summary>
/// <returns></returns>
qgsTreeItem* qgsTreeItem::parent()
{
	return parentItem
		;
}
qgsTreeItem* qgsTreeItem::child(int index)
{
	if (index < 0 || index >= childItems.size())
		return nullptr;
	return childItems.at(index);
}

int qgsTreeItem::childCount() const
{
	return childItems.count();
}

int qgsTreeItem::itemIndex() const
{
	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<qgsTreeItem*>(this));
	return 0;
}

int qgsTreeItem::columnCount() const
{
	return itemData.count();
}
/// <summary>
/// 数据
/// </summary>
/// <param name="column"></param>
/// <returns></returns>
QVariant qgsTreeItem::data(int column) const
{
	if (column < 0 || column >= itemData.size())
		return QVariant();
	return itemData.at(column);
}

bool qgsTreeItem::insertChildren(int position, int count, int columns) {
	if (position<0 || position>childItems.size())return false;

	for (int row = 0; row < count; row++)
	{
		QVector<QVariant> data(columns);
		qgsTreeItem* item = new qgsTreeItem(data, this);
		childItems.insert(position, item);
	}
	return true;
}
bool qgsTreeItem::insertColumns(int position, int columns) {
	if (position<0 || position>itemData.size())return false;

	for (int column = 0; column < columns; ++column)
		itemData.insert(position, QVariant());

	for (qgsTreeItem* child : qAsConst(childItems))
		child->insertColumns(position, columns);

	return true;
}
bool qgsTreeItem::removeChildren(int position, int count) {
	if (position<0 || position+count>childItems.size())return false;

	for (int row = 0; row < count; row++)
		delete childItems.takeAt(position); // take之后后续的item将往前排
	return true;
}
bool qgsTreeItem::removeColumns(int position, int columns) {
	if (position < 0 || position + columns > itemData.size())
		return false;

	for (int column = 0; column < columns; ++column)
		itemData.remove(position);

	for (qgsTreeItem* child : std::as_const(childItems))
		child->removeColumns(position, columns);

	return true;
}
bool qgsTreeItem::setData(int column, const QVariant& value) {
	if (column < 0 || column >= itemData.size())
		return false;
	itemData[column] = value;
	return true;
}