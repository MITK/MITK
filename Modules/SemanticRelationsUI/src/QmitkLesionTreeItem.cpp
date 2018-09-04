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

// semantic relations UI module
#include "QmitkLesionTreeItem.h"

// qt
#include <QStringList>

QmitkLesionTreeItem::QmitkLesionTreeItem(mitk::LesionData lesionData/* = mitk::LesionTreeItemData()*/)
{
  m_ItemData = lesionData;
}

QmitkLesionTreeItem::~QmitkLesionTreeItem()
{
  // nothing here
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
