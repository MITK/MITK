/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations UI module
#include "QmitkLesionTreeItem.h"

// qt
#include <QStringList>

QmitkLesionTreeItem::QmitkLesionTreeItem(mitk::LesionData lesionData/* = mitk::LesionTreeItemData()*/)
{
  m_ItemData = lesionData;
}

void QmitkLesionTreeItem::SetParent(ParentPointer parent)
{
  m_ParentItem = parent;
}

int QmitkLesionTreeItem::GetRow() const
{
  if (m_ParentItem.expired())
  {
    return 0;
  }

  auto parentItem = m_ParentItem.lock();
  return parentItem->GetRowOfChild(this->shared_from_this());
}

int QmitkLesionTreeItem::GetRowOfChild(ChildConstPointer child) const
{
  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it == m_Children.end())
  {
    return -1;
  }
  else
  {
    return std::distance(m_Children.begin(), it);
  }
}

void QmitkLesionTreeItem::AddChild(ChildPointer child)
{
  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it == m_Children.end())
  {
    // child does not already exist; add to vector of children
    m_Children.push_back(child);
    // add parent item
    child->SetParent(this->shared_from_this());
  }
}

void QmitkLesionTreeItem::RemoveChild(ChildPointer child)
{
  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it != m_Children.end())
  {
    m_Children.erase(it);
  }
}

void QmitkLesionTreeItem::SetData(const mitk::LesionData& lesionData)
{
  m_ItemData = lesionData;
}
