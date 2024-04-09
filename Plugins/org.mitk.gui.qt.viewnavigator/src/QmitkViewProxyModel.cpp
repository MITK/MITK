/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkViewProxyModel.h"

#include "QmitkCategoryItem.h"
#include "QmitkViewItem.h"

#include <QStandardItemModel>

QmitkViewProxyModel::QmitkViewProxyModel(QObject* parent)
  : QSortFilterProxyModel(parent)
{
  this->setFilterCaseSensitivity(Qt::CaseInsensitive);
  this->setSortCaseSensitivity(Qt::CaseInsensitive);
}

QmitkViewProxyModel::~QmitkViewProxyModel()
{
}

bool QmitkViewProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
  // Primarily filter view items. Include their parent category items, though. Consider only
  // views of visible categories.

  auto model = dynamic_cast<QStandardItemModel*>(this->sourceModel());

  if (model == nullptr)
    return true;

  const auto re = this->filterRegularExpression();

  auto index = model->index(sourceRow, 0, sourceParent);
  const auto* item = model->itemFromIndex(index);

  if (auto viewItem = dynamic_cast<const QmitkViewItem*>(item); viewItem != nullptr)
  {
    if (auto categoryItem = dynamic_cast<const QmitkCategoryItem*>(viewItem->parent()); categoryItem != nullptr)
    {
      if (!categoryItem->GetVisible())
        return false;
    }

    return viewItem->Match(re);
  }
  else if (auto categoryItem = dynamic_cast<const QmitkCategoryItem*>(item); categoryItem != nullptr)
  {
    if (!categoryItem->GetVisible())
      return false;

    return categoryItem->HasMatch(re);
  }

  return true;
}

bool QmitkViewProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  // Sort by item text in ascending order.

  auto model = this->sourceModel();
  auto leftText = model->data(left).toString();
  auto rightText = model->data(right).toString();
  auto caseSensitivity = this->sortCaseSensitivity();

  return leftText.compare(rightText, caseSensitivity) > 0;
}
