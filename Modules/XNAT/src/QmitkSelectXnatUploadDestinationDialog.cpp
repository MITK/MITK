/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSelectXnatUploadDestinationDialog.h"
#include "ui_QmitkSelectXnatUploadDestinationDialog.h"

#include <ctkXnatObject.h>
#include <ctkXnatResource.h>
#include <ctkXnatResourceFolder.h>
#include <ctkXnatSession.h>

#include "QmitkXnatTreeModel.h"

#include <QModelIndex>

QmitkSelectXnatUploadDestinationDialog::QmitkSelectXnatUploadDestinationDialog(ctkXnatSession *session,
                                                                               const QStringList &availableResources,
                                                                               QWidget *parent)
  : QDialog(parent),
    m_TreeModel(new QmitkXnatTreeModel),
    m_Url(""),
    m_ResourceName(""),
    m_CreateNewFolder(false),
    ui(new Ui::QmitkSelectXnatUploadDestinationDialog)
{
  ui->setupUi(this);

  m_TreeModel->addDataModel(session->dataModel());
  ui->treeView->setModel(m_TreeModel);

  connect(ui->cbSelectResources,
          SIGNAL(currentIndexChanged(const QString &)),
          this,
          SLOT(OnResourceSelected(const QString &)));
  connect(ui->treeView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(OnXnatNodeSelected(const QModelIndex &)));
  connect(ui->rbSelectResource, SIGNAL(toggled(bool)), this, SLOT(OnSelectResource(bool)));
  connect(ui->rbSelectFromTreeView, SIGNAL(toggled(bool)), this, SLOT(OnSelectFromTreeView(bool)));
  connect(ui->leResourceName, SIGNAL(textChanged(const QString &)), this, SLOT(OnResourceEntered(const QString &)));
  connect(ui->pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()));
  connect(ui->pbUpload, SIGNAL(clicked()), this, SLOT(OnUpload()));

  // Initial Setup
  ui->pbUpload->setEnabled(false);
  ui->leResourceName->setVisible(false);
  ui->treeView->hide();

  if (availableResources.isEmpty())
  {
    ui->rbSelectFromTreeView->setChecked(true);
    ui->rbSelectResource->setEnabled(false);
  }
  else
  {
    foreach (QString resourceName, availableResources)
    {
      ui->cbSelectResources->addItem(resourceName);
    }
    ui->cbSelectResources->addItem("Create new resource folder...");
  }
}

QmitkSelectXnatUploadDestinationDialog::~QmitkSelectXnatUploadDestinationDialog()
{
  delete ui;
}

void QmitkSelectXnatUploadDestinationDialog::OnCancel()
{
  this->done(QDialog::Rejected);
}

void QmitkSelectXnatUploadDestinationDialog::OnUpload()
{
  this->done(QDialog::Accepted);
}

void QmitkSelectXnatUploadDestinationDialog::OnSelectResource(bool selectResource)
{
  ui->pbUpload->setEnabled(false);
  ui->cbSelectResources->setVisible(selectResource);
  ui->leResourceName->setVisible(!selectResource);
  ui->treeView->setVisible(!selectResource);
  if (selectResource)
    ui->cbSelectResources->setCurrentIndex(0);
}

void QmitkSelectXnatUploadDestinationDialog::OnSelectFromTreeView(bool selectFromTreeView)
{
  ui->pbUpload->setEnabled(false);
  ui->cbSelectResources->setVisible(!selectFromTreeView);
  ui->leResourceName->setVisible(!selectFromTreeView);
  ui->treeView->setVisible(selectFromTreeView);
}

void QmitkSelectXnatUploadDestinationDialog::SetXnatResourceFolderUrl(const QString &url)
{
  m_Url = url;
}

ctkXnatObject *QmitkSelectXnatUploadDestinationDialog::GetUploadDestination()
{
  if (ui->rbSelectResource->isChecked() && !m_Url.isEmpty() && !m_ResourceName.isEmpty())
  {
    ctkXnatObject *selectedXnatObject = m_TreeModel->GetXnatObjectFromUrl(m_Url);

    ctkXnatResource *resource = new ctkXnatResource();
    resource->setName(m_ResourceName);
    resource->setParent(selectedXnatObject);
    if (!resource->exists())
      resource->save();
    return resource;
  }
  else
  {
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    return m_TreeModel->xnatObject(index);
  }
}

void QmitkSelectXnatUploadDestinationDialog::OnResourceSelected(const QString &resource)
{
  if (resource.isEmpty())
    ui->pbUpload->setEnabled(false);

  if (resource != "Create new resource folder...")
  {
    ui->pbUpload->setEnabled(true);
    m_ResourceName = resource;
    ui->leResourceName->hide();
  }
  else if (resource == "Create new resource folder...")
  {
    ui->pbUpload->setEnabled(false);
    ui->leResourceName->show();
  }
}

void QmitkSelectXnatUploadDestinationDialog::OnResourceEntered(const QString &resourceEntered)
{
  m_CreateNewFolder = !resourceEntered.isEmpty();
  ui->pbUpload->setEnabled(m_CreateNewFolder);
  m_ResourceName = resourceEntered;
}

void QmitkSelectXnatUploadDestinationDialog::OnXnatNodeSelected(const QModelIndex &index)
{
  if (!index.isValid())
    return;

  ctkXnatObject *selectedObject = m_TreeModel->xnatObject(index);
  ui->pbUpload->setEnabled(dynamic_cast<ctkXnatResource *>(selectedObject) != nullptr);
}
