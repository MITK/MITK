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

#include "QmitkPropertyItemSortFilterProxyModel.h"

QmitkPropertyItemSortFilterProxyModel::QmitkPropertyItemSortFilterProxyModel(QObject* parent)
  : QSortFilterProxyModel(parent)
{
}

QmitkPropertyItemSortFilterProxyModel::~QmitkPropertyItemSortFilterProxyModel()
{
}

bool QmitkPropertyItemSortFilterProxyModel::FilterAcceptsAnyChildRow(const QModelIndex& sourceParent) const
{
  QString propertyName = this->sourceModel()->data(sourceParent).toString();

  if (propertyName.contains(filterRegExp()))
    return true;

  if (this->sourceModel()->hasChildren(sourceParent))
  {
    for (int row = 0; row < this->sourceModel()->rowCount(sourceParent); ++row)
    {
      if(this->FilterAcceptsAnyChildRow(this->sourceModel()->index(row, 0, sourceParent)))
        return true;
    }
  }

  return false;
}

bool QmitkPropertyItemSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
  return this->FilterAcceptsAnyChildRow(this->sourceModel()->index(sourceRow, 0, sourceParent));
}

bool QmitkPropertyItemSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  QString leftString = this->sourceModel()->data(left).toString();
  QString rightString = this->sourceModel()->data(right).toString();

  return leftString.compare(rightString, this->sortCaseSensitivity()) < 0;
}
