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

#include "QmitkPropertyTreeFilterProxyModel.h"
#include <mitkLogMacros.h>

QmitkPropertyTreeFilterProxyModel::QmitkPropertyTreeFilterProxyModel(QObject *parent)
: QSortFilterProxyModel(parent)
{
}

QmitkPropertyTreeFilterProxyModel::~QmitkPropertyTreeFilterProxyModel()
{
}

bool QmitkPropertyTreeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  return filterAcceptsAnyChildRows(sourceModel()->index(sourceRow, 0, sourceParent));
}

bool QmitkPropertyTreeFilterProxyModel::filterAcceptsAnyChildRows(const QModelIndex &sourceParent) const
{
  QString propertyName = sourceModel()->data(sourceParent).toString();

  if (propertyName.contains(filterRegExp()))
    return true;

  if (sourceModel()->hasChildren(sourceParent))
  {
    for (int row = 0; row < sourceModel()->rowCount(sourceParent); ++row)
    {
      if (filterAcceptsAnyChildRows(sourceModel()->index(row, 0, sourceParent)))
        return true;
    }
  }

  return false;
}

bool QmitkPropertyTreeFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  return sourceModel()->data(left).toString().compare(sourceModel()->data(right).toString(), Qt::CaseInsensitive) > 0;
}
