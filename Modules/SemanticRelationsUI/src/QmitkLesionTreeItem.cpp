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

QmitkLesionTreeItem::QmitkLesionTreeItem(mitk::SemanticTypes::ID lesionUID)
{
  m_LesionID = lesionUID;
}

QmitkLesionTreeItem::~QmitkLesionTreeItem()
{
  // nothing here
}

QmitkLesionTreeItem::ChildPointer QmitkLesionTreeItem::GetChildInRow(int row) const
{
  return m_Children.at(row);
}

int QmitkLesionTreeItem::GetRow() const
{
  if (m_ParentItem.expired())
  {
    return 0;
  }

  auto it = std::find(m_Children.begin(), m_Children.end(), this->shared_from_this());
  if (it == m_Children.end())
  {
    return -1;
  }
  else
  {
    return std::distance(m_Children.begin(), it);
  }
}

size_t QmitkLesionTreeItem::ChildCount() const
{
  return m_Children.size();
}

void QmitkLesionTreeItem::AddChild(ChildPointer child)
{
  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it == m_Children.end())
  {
    // child does not already exist; add to vector of children
    m_Children.push_back(child);
    m_ParentItem = this->shared_from_this();
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

void QmitkLesionTreeItem::SetData(const mitk::SemanticTypes::Lesion& value)
{
  m_ItemData.SetLesion(value);
}
