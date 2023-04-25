/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelTreeView.h>

#include <QmitkMultiLabelTreeModel.h>

QmitkMultiLabelTreeView::QmitkMultiLabelTreeView(QWidget* parent) : QTreeView(parent)
{
}

QItemSelectionModel::SelectionFlags QmitkMultiLabelTreeView::selectionCommand(const QModelIndex& index, const QEvent* event) const
{
  auto value = index.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);
  if (index.column()!=0 || !value.isValid())
  {
    return QItemSelectionModel::NoUpdate;
  }

  return   QAbstractItemView::selectionCommand(index, event);
}

