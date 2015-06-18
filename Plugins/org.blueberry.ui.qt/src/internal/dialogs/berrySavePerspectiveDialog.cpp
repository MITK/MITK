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

#include "berrySavePerspectiveDialog.h"
#include "berryPerspectiveListModel.h"
#include "internal/berryPerspectiveRegistry.h"

#include "ui_berrySavePerspectiveDialog.h"

#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QPushButton>

namespace berry {

SavePerspectiveDialog::SavePerspectiveDialog(PerspectiveRegistry& perspReg, QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::SavePerspectiveDialog)
  , model(new PerspectiveListModel(perspReg, true, this))
  , perspReg(perspReg)
{
  ui->setupUi(this);

  auto   proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setSourceModel(model);
  proxyModel->sort(0);
  ui->listView->setModel(proxyModel);
  ui->listView->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->listView->setIconSize(QSize(16, 16));

  connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(PerspectiveNameChanged(QString)));
  connect(ui->listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(PerspectiveSelected(QItemSelection,QItemSelection)));

  this->UpdateButtons();
  ui->lineEdit->setFocus();
}

SavePerspectiveDialog::~SavePerspectiveDialog()
{
  delete ui;
}

void SavePerspectiveDialog::SetInitialSelection(const SmartPointer<IPerspectiveDescriptor>& initialSelection)
{
  if (initialSelection.IsNotNull() &&
      ui->listView->selectionModel()->selection().empty())
  {
    QModelIndex index = this->model->index(initialSelection->GetId());
    if (index.isValid())
    {
      ui->listView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    }
  }
}

SmartPointer<IPerspectiveDescriptor> SavePerspectiveDialog::GetPersp() const
{
  return persp;
}

QString SavePerspectiveDialog::GetPerspName() const
{
  return perspName;
}

void SavePerspectiveDialog::accept()
{
  perspName = ui->lineEdit->text();
  persp = perspReg.FindPerspectiveWithLabel(perspName);
  if (persp.IsNotNull())
  {
    // Confirm ok to overwrite
    QString msg = QString("A perspective with the name \"%1\" already exists. Do you want to overwrite?").arg(perspName);
    int ret = QMessageBox::question(this,
                                    "Overwrite Perspective",
                                    msg,
                                    QMessageBox::No | QMessageBox::Cancel | QMessageBox::Yes,
                                    QMessageBox::No);

    switch (ret)
    {
    case QMessageBox::Yes: //yes
      break;
    case QMessageBox::No: //no
      return;
    case QMessageBox::Cancel: //cancel
      this->reject();
      return;
    default:
      return;
    }
  }

  QDialog::accept();
}

void SavePerspectiveDialog::PerspectiveNameChanged(const QString& text)
{
  perspName = text;
  persp = perspReg.FindPerspectiveWithLabel(perspName);
  if (persp.IsNull())
  {
    ui->listView->selectionModel()->clearSelection();
  }
  else
  {
    QModelIndex selIndex = model->index(persp->GetId());
    ui->listView->selectionModel()->select(selIndex, QItemSelectionModel::ClearAndSelect);
  }

  this->UpdateButtons();
}

void SavePerspectiveDialog::PerspectiveSelected(const QItemSelection& selected, const QItemSelection& /*deselected*/)
{
  persp = nullptr;
  if (!selected.isEmpty())
  {
    persp = model->perspectiveDescriptor(selected.indexes().front());
  }

  if (!persp.IsNull())
  {
    perspName = persp->GetLabel();
    ui->lineEdit->setText(perspName);
  }

  this->UpdateButtons();
}

void SavePerspectiveDialog::UpdateButtons()
{
  QString label = ui->lineEdit->text();
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(perspReg.ValidateLabel(label));
}

}
