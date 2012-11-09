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

#include "QmitkPropertyTreeItem.h"
#include <QStringList>

QmitkPropertyTreeItem::QmitkPropertyTreeItem(const QList<QVariant> &data)
: m_Data(data),
  m_Parent(NULL)
{
}

QmitkPropertyTreeItem::~QmitkPropertyTreeItem()
{
  qDeleteAll(m_Children);
}

void QmitkPropertyTreeItem::AppendChild(QmitkPropertyTreeItem *child)
{
  if (child == NULL)
    return;

  // If property name contains no full stop we can append the property directly.
  if (!child->GetData(0).toString().contains('.'))
  {
    m_Children.append(child);
    child->m_Parent = this;
  }
  else
  {
    // Property name contains full stop(s). We split the name appropriately.
    QStringList names = child->GetData(0).toString().split('.');

    // Traverse the subtree and insert items if they are not already present.
    QmitkPropertyTreeItem *parent_ = this;

    for (int i = 0; i < names.count(); ++i)
    {
      // Subtree present/recently created, append actual property as a leaf item
      if (i == names.count() - 1)
      {
        QList<QVariant> data;
        data << names[i] << child->m_Data[1];
        parent_->AppendChild(new QmitkPropertyTreeItem(data));

        delete child;
        child = NULL;
      }
      else
      {
        QmitkPropertyTreeItem *child_ = NULL;

        for (int j = 0; j < parent_->m_Children.count(); ++j)
        {
          if (parent_->m_Children[j]->GetData(0).toString() == names[i])
          {
            child_ = parent_->m_Children[j];
            break;
          }
        }

        if (child_ == NULL)
        {
          QList<QVariant> data;
          data << names[i] << QVariant();
          child_ = new QmitkPropertyTreeItem(data);
          parent_->AppendChild(child_);
        }

        parent_ = child_;
      }
    }
  }
}

QmitkPropertyTreeItem * QmitkPropertyTreeItem::GetChild(int row)
{
  return m_Children.value(row);
}

int QmitkPropertyTreeItem::GetChildCount() const
{
  return m_Children.count();
}

int QmitkPropertyTreeItem::GetColumnCount() const
{
  return m_Data.count();
}

QVariant QmitkPropertyTreeItem::GetData(int column) const
{
  return m_Data.value(column);
}

QmitkPropertyTreeItem * QmitkPropertyTreeItem::GetParent()
{
  return m_Parent;
}

int QmitkPropertyTreeItem::GetRow() const
{
  if (m_Parent != NULL)
    return m_Parent->m_Children.indexOf(const_cast<QmitkPropertyTreeItem *>(this));

  return 0;
}
