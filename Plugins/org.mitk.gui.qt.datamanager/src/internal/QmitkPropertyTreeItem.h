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

#ifndef QMITKPROPERTYTREEITEM_H
#define QMITKPROPERTYTREEITEM_H

#include <QList>
#include <QVariant>

class QmitkPropertyTreeItem
{
public:
  explicit QmitkPropertyTreeItem(const QList<QVariant> &data);
  ~QmitkPropertyTreeItem();

  void AppendChild(QmitkPropertyTreeItem *child);
  QmitkPropertyTreeItem * GetChild(int row);
  int GetChildCount() const;
  int GetColumnCount() const;
  QVariant GetData(int column) const;
  QmitkPropertyTreeItem * GetParent();
  int GetRow() const;

private:
  QList<QmitkPropertyTreeItem *> m_Children;
  QList<QVariant> m_Data;
  QmitkPropertyTreeItem *m_Parent;
};

#endif
