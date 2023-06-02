#pragma once
#include <QVector>
#include <QVariant>

/// <summary>
/// ͨ����״�б�ڵ���
/// </summary>
class qgsTreeItem
{

public :
	explicit qgsTreeItem(const QVector<QVariant>& data, qgsTreeItem* parent = nullptr);
	~qgsTreeItem();
	/// <summary>
	/// ָ��λ�õ�����
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	qgsTreeItem* child(int index);
	/// <summary>
	/// ������
	/// </summary>
	/// <returns></returns>
	int childCount() const;
	/// <summary>
	/// �ֶ���
	/// </summary>
	/// <returns></returns>
	int columnCount() const;
	/// <summary>
	/// ����
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

