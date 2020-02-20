/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertyItem_h
#define QmitkPropertyItem_h

#include <QList>
#include <QVariant>

class QmitkPropertyItem
{
public:
  explicit QmitkPropertyItem(const QList<QVariant> &data);
  ~QmitkPropertyItem();

  void AppendChild(QmitkPropertyItem *child);
  QmitkPropertyItem *GetChild(int row) const;
  int GetChildCount() const;
  int GetColumnCount() const;
  QVariant GetData(int column) const;
  QmitkPropertyItem *GetParent() const;
  int GetRow();

private:
  QmitkPropertyItem(const QmitkPropertyItem &);
  QmitkPropertyItem &operator=(const QmitkPropertyItem &);

  QList<QVariant> m_Data;
  QList<QmitkPropertyItem *> m_Children;
  QmitkPropertyItem *m_Parent;
};

#endif
