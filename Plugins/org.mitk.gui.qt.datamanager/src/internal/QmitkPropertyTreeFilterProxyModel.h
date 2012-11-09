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

#ifndef QMITKPROPERTYTREEFILTERPROXYMODEL_H
#define QMITKPROPERTYTREEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class QmitkPropertyTreeFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  QmitkPropertyTreeFilterProxyModel(QObject *parent = NULL);
  ~QmitkPropertyTreeFilterProxyModel();

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
  bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
  bool filterAcceptsAnyChildRows(const QModelIndex &sourceParent) const;
};

#endif
