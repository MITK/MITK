/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKLESIONTREEITEM_H
#define QMITKLESIONTREEITEM_H

// mitk semantic relations UI
#include "mitkLesionData.h"

// mitk semantic relations
#include <mitkSemanticTypes.h>

// qt
#include <QVariant>

// c++
#include <memory>
#include <vector>

/*
* @brief This class is used by custom tree models to create their tree items.
*        It provides functions to traverse and modify the tree.
*        Additionally it holds some 'LesionData' that is used to display lesion properties inside a tree view.
*/
class QmitkLesionTreeItem : public std::enable_shared_from_this<QmitkLesionTreeItem>
{
public:

  using ChildPointer = std::shared_ptr<QmitkLesionTreeItem>;
  using ChildConstPointer = std::shared_ptr<const QmitkLesionTreeItem>;
  using ParentPointer = std::weak_ptr<QmitkLesionTreeItem>;

  QmitkLesionTreeItem(mitk::LesionData lesionData = mitk::LesionData());

  /**
  * @brief Return the child of this item at a specific position.
  *
  * @param row   Determines the position of a child item to return.
  *
  * @return The child of this item at a specific position.
  */
  ChildPointer GetChildInRow(int row) const { return m_Children.at(row); };
  /**
  * @brief Return the parent item.
  *
  * @return The parent item as std::weak_ptr.
  */
  ParentPointer GetParent() const { return m_ParentItem; };
  /**
  * @brief Set the parent item of this item.
  *
  * @param parent   The new parent item of this item.
  */
  void SetParent(ParentPointer parent);
  /**
  * @brief Return the item data, which contains ...
  *
  *   see <code>mitk::LesionItemData</code>
  */
  mitk::LesionData& GetData() { return m_ItemData; };
  /**
  * @brief Get the row of this item relative to its parent item using 'GetRowOfChild'.
  *
  * @return The row of this item relative to its parent item.
  */
  int GetRow() const;
  /**
  * @brief Get the row of the given child item relative to this item.
  *
  * @param child    The child item whose row is to be determined.
  *
  * @return The row of the child item.
  */
  int GetRowOfChild(ChildConstPointer child) const;
  /**
  * @brief Return the number of child items.
  *
  * @return Number of child items.
  */
  size_t ChildCount() const { return m_Children.size(); };
  /**
  * @brief Add a new child to the list of children of this item if it is not already a child item.
  *
  * @param child   The child item to add to this item.
  */
  void AddChild(ChildPointer child);
  /**
  * @brief Remove a child from the list of children of this item.
  *
  * @param child   The child item to remove from this item.
  */
  void RemoveChild(ChildPointer child);
  /**
  * @brief Set the item data of this item.
  *
  * @param lesionData LesionData that provides information about this item.
  */
  void SetData(const mitk::LesionData& lesionData);

private:

  ParentPointer m_ParentItem;
  std::vector<ChildPointer> m_Children;
  mitk::LesionData m_ItemData;

};

Q_DECLARE_METATYPE(QmitkLesionTreeItem)
Q_DECLARE_METATYPE(QmitkLesionTreeItem*)

#endif // QMITKLESIONTREEITEM_H
