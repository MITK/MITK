/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKLESIONTREEITEM_H
#define QMITKLESIONTREEITEM_H

// mitk semantic relations UI
#include "mitkLesionItemData.h"

// mitk semantic relations
#include <mitkSemanticTypes.h>

// c++
#include <memory>
#include <vector>

/*
* @brief
*/
class QmitkLesionTreeItem : public std::enable_shared_from_this<QmitkLesionTreeItem>
{
public:

  using ChildPointer = std::shared_ptr<QmitkLesionTreeItem>;
  using ParentPointer = std::weak_ptr<QmitkLesionTreeItem>;

  QmitkLesionTreeItem(mitk::SemanticTypes::ID lesionUID);
  ~QmitkLesionTreeItem();

  /**
  * @brief Return the child of this item at a specific position.
  *
  * @param row   Determines the position of a child item to return.
  */
  ChildPointer GetChildInRow(int row) const;
  /**
  * @brief Return the parent item.
  */
  ParentPointer GetParent() const { return m_ParentItem; };

  /**
  * @brief Return the item data, which contains ...
  *
  *   see <code>mitk::LesionItemData</code>
  */
  mitk::LesionItemData& GetData() { return m_ItemData; };

  mitk::SemanticTypes::ID GetLesionID() { return m_LesionID; };
  /**
  * @brief 
  *
  *
  */
  int GetRow() const;
  /**
  * @brief 
  *
  *
  */
  size_t ChildCount() const;
  /**
  * @brief
  *
  *
  */
  void AddChild(ChildPointer child);
  /**
  * @brief 
  *
  *
  */
  void RemoveChild(ChildPointer child);
  /**
  * @brief Set the item data of this item.
  *
  * @param value   LesionItemData that provides information about this item.
  */
  void SetData(const mitk::SemanticTypes::Lesion& value);

private:

  ParentPointer m_ParentItem;
  std::vector<ChildPointer> m_Children;
  mitk::LesionItemData m_ItemData;

  mitk::SemanticTypes::ID m_LesionID;
};

#endif // QMITKLESIONTREEITEM_H
