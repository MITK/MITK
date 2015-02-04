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


#include "QmitkModulesDialog.h"

#include <QHBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QDialogButtonBox>

#include "QmitkModuleTableModel.h"

QmitkModulesDialog::QmitkModulesDialog(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f)
{
  this->setWindowTitle("MITK Modules");

  QVBoxLayout* layout = new QVBoxLayout();
  this->setLayout(layout);

  QTableView* tableView = new QTableView(this);
  QmitkModuleTableModel* tableModel = new QmitkModuleTableModel(tableView);
  QSortFilterProxyModel* sortProxyModel = new QSortFilterProxyModel(tableView);
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

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  tableView->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
  tableView->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
  tableView->horizontalHeader()->setResizeMode(5, QHeaderView::ResizeToContents);
#else
  tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
#endif
  tableView->horizontalHeader()->setStretchLastSection(true);
  tableView->horizontalHeader()->setCascadingSectionResizes(true);

  layout->addWidget(tableView);

  QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Close);
  layout->addWidget(btnBox);

  this->resize(800, 600);

  connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
}

