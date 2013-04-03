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

#ifndef QmitkPropertyItem_h
#define QmitkPropertyItem_h

#include <QList>
#include <QVariant>

class QmitkPropertyItem
{
public:
  explicit QmitkPropertyItem(const QList<QVariant>& data);
  ~QmitkPropertyItem();

  void AppendChild(QmitkPropertyItem* child);
  QmitkPropertyItem* GetChild(int row) const;
  int GetChildCount() const;
  int GetColumnCount() const;
  QVariant GetData(int column) const;
  QmitkPropertyItem* GetParent() const;
  int GetRow();

private:
  QmitkPropertyItem(const QmitkPropertyItem&);
  QmitkPropertyItem& operator=(const QmitkPropertyItem&);

  QList<QVariant> m_Data;
  QList<QmitkPropertyItem*> m_Children;
  QmitkPropertyItem* m_Parent;
};

#endif
