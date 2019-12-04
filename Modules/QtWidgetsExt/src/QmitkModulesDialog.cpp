/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkModulesDialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QTableView>

#include "QmitkModuleTableModel.h"

QmitkModulesDialog::QmitkModulesDialog(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
  this->setWindowTitle("MITK Modules");

  auto layout = new QVBoxLayout();
  this->setLayout(layout);

  auto tableView = new QTableView(this);
  auto tableModel = new QmitkModuleTableModel(tableView);
  auto sortProxyModel = new QSortFilterProxyModel(tableView);
  sortProxyModel->setSourceModel(tableModel);
  sortProxyModel->setDynamicSortFilter(true);
  tableView->setModel(sortProxyModel);

  tableView->verticalHeader()->hide();
  tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tableView->setTextElideMode(Qt::ElideMiddle);
  tableView->setSortingEnabled(true);
  tableView->sortByColumn(0, Qt::AscendingOrder);

  tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  tableView->horizontalHeader()->setStretchLastSection(true);
  tableView->horizontalHeader()->setCascadingSectionResizes(true);

  layout->addWidget(tableView);

  auto btnBox = new QDialogButtonBox(QDialogButtonBox::Close);
  layout->addWidget(btnBox);

  this->resize(800, 600);

  connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
}
