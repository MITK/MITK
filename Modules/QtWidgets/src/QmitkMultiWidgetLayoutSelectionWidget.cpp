/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkMultiWidgetLayoutSelectionWidget.h"

QmitkMultiWidgetLayoutSelectionWidget::QmitkMultiWidgetLayoutSelectionWidget(QWidget* parent/* = 0*/)
  : QWidget(parent)
{
  Init();
}

void QmitkMultiWidgetLayoutSelectionWidget::Init()
{
  ui.setupUi(this);
  connect(ui.tableWidget, &QTableWidget::itemSelectionChanged, this, &QmitkMultiWidgetLayoutSelectionWidget::OnTableItemSelectionChanged);
  connect(ui.setLayoutPushButton, &QPushButton::clicked, this, &QmitkMultiWidgetLayoutSelectionWidget::OnSetLayoutButtonClicked);
}

void QmitkMultiWidgetLayoutSelectionWidget::OnTableItemSelectionChanged()
{
  QItemSelectionModel* selectionModel = ui.tableWidget->selectionModel();

  int row = 0;
  int column = 0;
  QModelIndexList indices = selectionModel->selectedIndexes();
  if (indices.size() > 0)
  {
    row = indices[0].row();
    column = indices[0].column();

    QModelIndex topLeft = ui.tableWidget->model()->index(0, 0, QModelIndex());
    QModelIndex bottomRight = ui.tableWidget->model()->index(row, column, QModelIndex());

    QItemSelection cellSelection;
    cellSelection.select(topLeft, bottomRight);
    selectionModel->select(cellSelection, QItemSelectionModel::Select);
  }
}

void QmitkMultiWidgetLayoutSelectionWidget::OnSetLayoutButtonClicked()
{
  int row = 0;
  int column = 0;
  QModelIndexList indices = ui.tableWidget->selectionModel()->selectedIndexes();
  if (indices.size() > 0)
  {
    // find largest row and column
    for (const QModelIndex modelIndex : indices)
    {
      if (modelIndex.row() > row)
      {
        row = modelIndex.row();
      }
      if (modelIndex.column() > column)
      {
        column = modelIndex.column();
      }
    }

    close();
    emit LayoutSet(row+1, column+1);
  }
}
