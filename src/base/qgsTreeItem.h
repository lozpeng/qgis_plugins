#pragma once
#include <QVector>
#include <QVariant>

/// <summary>
/// 通用树状列表节点项
/// </summary>
class qgsTreeItem
{

public :
	explicit qgsTreeItem(const QVector<QVariant>& data, qgsTreeItem* parent = nullptr);
	~qgsTreeItem();
	/// <summary>
	/// 指定位置的子项
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	qgsTreeItem* child(int index);
	/// <summary>
	/// 子项数
	/// </summary>
	/// <returns></returns>
	int childCount() const;
	/// <summary>
	/// 字段数
	/// </summary>
	/// <returns></returns>
	int columnCount() const;
	/// <summary>
	/// 数据
	/// </summary>
	/// <param name="column"></param>
	/// <returns></returns>
	QVariant data(int column) const;

	bool insertChildren(int position, int count, int columns);
	bool insertColumns(int position, int columns);
	qgsTreeItem* parent();
	bool removeChildren(int position, int count);
	bool removeColumns(int position, int columns);

	int itemIndex() const;
	bool setData(int column, const QVariant& value);


private:
	QVector<qgsTreeItem*> childItems;
	QVector<QVariant> itemData;
	qgsTreeItem* parentItem;


};

