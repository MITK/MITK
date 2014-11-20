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


#include "QmitkModuleView.h"

#include <QmitkModuleTableModel.h>

#include <QTableView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QSettings>

QmitkModuleView::QmitkModuleView()
  : tableView(0)
{
}

void QmitkModuleView::SetFocus()
{
  //tableView->setFocus();
}

void QmitkModuleView::CreateQtPartControl(QWidget *parent)
{
  QHBoxLayout* layout = new QHBoxLayout();
  layout->setMargin(0);
  parent->setLayout(layout);

  tableView = new QTableView(parent);
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
  // Fixed size for "Id" column
  tableView->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
  // Fixed size for "Version" column
  tableView->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);
#else
  tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
#endif
  tableView->horizontalHeader()->setStretchLastSection(true);
  tableView->horizontalHeader()->setCascadingSectionResizes(true);

  layout->addWidget(tableView);

  if (viewState)
  {
    berry::IMemento::Pointer tableHeaderState = viewState->GetChild("tableHeader");
    if (tableHeaderState)
    {
      std::string key;
      tableHeaderState->GetString("qsettings-key", key);
      if (!key.empty())
      {
        QSettings settings;
        QByteArray ba = settings.value(QString::fromStdString(key)).toByteArray();
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
  tableHeaderState->PutString("qsettings-key", key.toStdString());
}
