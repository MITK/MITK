/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelSegmentationTreeView.h>

#include <QmitkMultiLabelSegmentationTreeModel.h>

QmitkMultiLabelSegmentationTreeView::QmitkMultiLabelSegmentationTreeView(QWidget* parent) : QTreeView(parent)
{
}

QItemSelectionModel::SelectionFlags QmitkMultiLabelSegmentationTreeView::selectionCommand(const QModelIndex& index, const QEvent* event) const
{
  auto value = index.data(QmitkMultiLabelSegmentationTreeModel::ItemModelRole::LabelValueRole);
  if (index.column()!=0 || !value.isValid())
  {
    return QItemSelectionModel::NoUpdate;
  }

  return   QAbstractItemView::selectionCommand(index, event);
}

