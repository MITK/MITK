/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQtShowPerspectiveDialog.h"
#include "ui_berryQtShowPerspectiveDialog.h"

#include <berryPerspectiveListModel.h>

#include <QSortFilterProxyModel>

namespace berry {

QtShowPerspectiveDialog::QtShowPerspectiveDialog(IPerspectiveRegistry* perspReg, QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::QtShowPerspectiveDialog)
{
  ui->setupUi(this);

  QAbstractItemModel* model = new PerspectiveListModel(*perspReg, true, this);

  QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setSourceModel(model);
  proxyModel->sort(0);

  ui->m_ListView->setModel(proxyModel);
  ui->m_ListView->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->m_ListView->selectionModel()->select(model->index(0, 0), QItemSelectionModel::ClearAndSelect);
  ui->m_ListView->setIconSize(QSize(16, 16));

  connect(ui->m_ListView, SIGNAL(clicked(QModelIndex)), this, SLOT(setDescription(QModelIndex)));
  connect(ui->m_ListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
  connect(ui->m_ListView, SIGNAL(activated(QModelIndex)), this, SLOT(accept()));

}

QtShowPerspectiveDialog::~QtShowPerspectiveDialog()
{
  delete ui;
}

QString QtShowPerspectiveDialog::GetSelection() const
{
  const QItemSelection selection = ui->m_ListView->selectionModel()->selection();
  if (selection.isEmpty()) return QString::null;

  return selection.indexes().front().data(PerspectiveListModel::Id).toString();
}

void QtShowPerspectiveDialog::setDescription(const QModelIndex& index)
{
  ui->m_Description->setText(ui->m_ListView->model()->data(index, PerspectiveListModel::Description).toString());
}

}
