/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkXnatUploadFromDataStorageDialog.h"
#include "ui_QmitkXnatUploadFromDataStorageDialog.h"

#include <mitkDataStorage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

QmitkXnatUploadFromDataStorageDialog::QmitkXnatUploadFromDataStorageDialog(QWidget *parent)
  : QDialog(parent), ui(new Ui::QmitkXnatUploadFromDataStorageDialog), m_SelectedNode(nullptr)
{
  ui->setupUi(this);

  // Not yet implemented
  ui->cbUploadMITKProject->hide();
  ui->leMITKProjectFileName->hide();
  //

  connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(OnCancel()));
  connect(ui->btnUpload, SIGNAL(clicked()), this, SLOT(OnUpload()));
  connect(ui->cbUploadMITKProject, SIGNAL(checked()), this, SLOT(OnUploadSceneChecked()));
  connect(ui->leMITKProjectFileName,
          SIGNAL(textEdited(const QString &)),
          this,
          SLOT(OnMITKProjectFileNameEntered(const QString &)));
  connect(ui->cBDataSelection,
          SIGNAL(OnSelectionChanged(const mitk::DataNode *)),
          this,
          SLOT(OnDataSelected(const mitk::DataNode *)));

  mitk::NodePredicateNot::Pointer noHelper =
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"));
  mitk::NodePredicateNot::Pointer noHidden =
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object"));
  mitk::NodePredicateAnd::Pointer pred = mitk::NodePredicateAnd::New(noHelper, noHidden);
  ui->cBDataSelection->SetPredicate(pred);
}

QmitkXnatUploadFromDataStorageDialog::~QmitkXnatUploadFromDataStorageDialog()
{
  delete ui;
}

void QmitkXnatUploadFromDataStorageDialog::SetDataStorage(mitk::DataStorage *ds)
{
  ui->cBDataSelection->SetDataStorage(ds);
}

void QmitkXnatUploadFromDataStorageDialog::OnUpload()
{
  this->done(QDialog::Accepted);
}

void QmitkXnatUploadFromDataStorageDialog::OnUploadSceneChecked()
{
}

void QmitkXnatUploadFromDataStorageDialog::OnCancel()
{
  this->done(QDialog::Rejected);
}

void QmitkXnatUploadFromDataStorageDialog::OnMITKProjectFileNameEntered(const QString & /*text*/)
{
}

void QmitkXnatUploadFromDataStorageDialog::OnDataSelected(const mitk::DataNode *node)
{
  m_SelectedNode = const_cast<mitk::DataNode *>(node);
  if (m_SelectedNode.IsNotNull())
    ui->btnUpload->setEnabled(true);
}

mitk::DataNode::Pointer QmitkXnatUploadFromDataStorageDialog::GetSelectedNode()
{
  return m_SelectedNode;
}
