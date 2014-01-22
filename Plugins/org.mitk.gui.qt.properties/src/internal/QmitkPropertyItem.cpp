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

#include "QmitkPropertyItem.h"
#include <QStringList>

QmitkPropertyItem::QmitkPropertyItem(const QList<QVariant>& data)
  : m_Data(data),
    m_Parent(NULL)
{
}

QmitkPropertyItem::~QmitkPropertyItem()
{
  qDeleteAll(m_Children);
}

void QmitkPropertyItem::AppendChild(QmitkPropertyItem* child)
{
  if (child == NULL)
    return;

  // If property name doesn't contain period(s), append property directly, but...
  if (!child->GetData(0).toString().contains('.'))
  {
    // ... if node already exists without property, just attach it.
    bool nodeAlreadyExists = false;

    for (int i = 0; i < m_Children.count(); ++i)
    {
      if (m_Children[i]->GetData(0).toString() == child->GetData(0).toString())
      {
        if (m_Children[i]->GetData(1).isNull() && child->GetData(1).isValid())
          m_Children[i]->m_Data[1] = child->m_Data[1];

        nodeAlreadyExists = true;
        break;
      }
    }

    if (!nodeAlreadyExists)
    {
      m_Children.append(child);
      child->m_Parent = this;
    }
  }
  else
  {
    // Property name contains period(s). Split accordingly.
    QStringList names = child->GetData(0).toString().split('.');

    // Traverse subtree and insert nodes if not already present.
    QmitkPropertyItem* currentParent = this;

    for (int i = 0; i < names.count(); ++i)
    {
      if (i != names.count() - 1)
      {
        QmitkPropertyItem* currentChild = NULL;

        // Search for current node.
        for (int j = 0; j < currentParent->m_Children.count(); ++j)
        {
          if (currentParent->m_Children[j]->GetData(0).toString() == names[i])
          {
            currentChild = currentParent->m_Children[j];
            break;
          }
        }

        // Node doesn't exist. Create and append it.
        if (currentChild == NULL)
        {
          QList<QVariant> data;
          data << names[i] << QVariant();

          currentChild = new QmitkPropertyItem(data);
          currentParent->AppendChild(currentChild);
        }

        currentParent = currentChild;
      }
      else
      {
        // Subtree already present, append property as leaf node.
        QList<QVariant> data;
        data << names[i] << child->m_Data[1];

        currentParent->AppendChild(new QmitkPropertyItem(data));

        delete child;
        child = NULL;
      }
    }
  }
}

QmitkPropertyItem* QmitkPropertyItem::GetChild(int row) const
{
  return m_Children.value(row);
}

int QmitkPropertyItem::GetChildCount() const
{
  return m_Children.count();
}

int QmitkPropertyItem::GetColumnCount() const
{
  return m_Data.count();
}

QVariant QmitkPropertyItem::GetData(int column) const
{
  return m_Data.value(column);
}

QmitkPropertyItem* QmitkPropertyItem::GetParent() const
{
  return m_Parent;
}

int QmitkPropertyItem::GetRow()
{
  if (m_Parent != NULL)
    return m_Parent->m_Children.indexOf(this);

  return 0;
}
