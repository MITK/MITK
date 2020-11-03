/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkModuleView.h"

#include <QmitkModuleTableModel.h>

#include <QTableView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QSettings>

QmitkModuleView::QmitkModuleView()
  : tableView(nullptr)
{
}

void QmitkModuleView::SetFocus()
{
  //tableView->setFocus();
}

void QmitkModuleView::CreateQtPartControl(QWidget *parent)
{
  auto   layout = new QHBoxLayout();
  layout->setMargin(0);
  parent->setLayout(layout);

  tableView = new QTableView(parent);
  auto   tableModel = new QmitkModuleTableModel(tableView);
  auto   sortProxyModel = new QSortFilterProxyModel(tableView);
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

  if (viewState)
  {
    berry::IMemento::Pointer tableHeaderState = viewState->GetChild("tableHeader");
    if (tableHeaderState)
    {
      QString key;
      tableHeaderState->GetString("qsettings-key", key);
      if (!key.isEmpty())
      {
        QSettings settings;
        QByteArray ba = settings.value(key).toByteArray();
        tableView->horizontalHeader()->restoreState(ba);
      }
    }
  }
}

void QmitkModuleView::Init(berry::IViewSite::Pointer site, berry::IMemento::Pointer memento)
{
  berry::QtViewPart::Init(site, memento);
  viewState = memento;
}

void QmitkModuleView::SaveState(berry::IMemento::Pointer memento)
{
  QString key = "QmitkModuleView_tableHeader";
  QByteArray ba = tableView->horizontalHeader()->saveState();
  QSettings settings;
  settings.setValue(key, ba);

  berry::IMemento::Pointer tableHeaderState = memento->CreateChild("tableHeader");
  tableHeaderState->PutString("qsettings-key", key);
}
