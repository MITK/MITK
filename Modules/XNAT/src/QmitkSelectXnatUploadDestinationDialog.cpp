#include "QmitkSelectXnatUploadDestinationDialog.h"
#include "ui_QmitkSelectXnatUploadDestinationDialog.h"

#include <ctkXnatObject.h>
#include <ctkXnatResource.h>
#include <ctkXnatSession.h>

#include "QmitkXnatTreeModel.h"

#include <QModelIndex>

QmitkSelectXnatUploadDestinationDialog::QmitkSelectXnatUploadDestinationDialog(ctkXnatSession* session, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::QmitkSelectXnatUploadDestinationDialog)
{
  ui->setupUi(this);

  m_TreeModel = new QmitkXnatTreeModel();
  m_TreeModel->addDataModel(session->dataModel());
  ui->treeView->setModel(m_TreeModel);

  connect(ui->cbSelectResources, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnResourceSelected(const QString&)));
  connect(ui->treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnXnatNodeSelected(const QModelIndex&)));
  connect(ui->rbSelectResource, SIGNAL(toggled(bool)), this, SLOT(OnSelectResource(bool)));
  connect(ui->rbSelectFromTreeView, SIGNAL(toggled(bool)), this, SLOT(OnSelectFromTreeView(bool)));
  connect(ui->leResourceName, SIGNAL(textChanged(const QString&)), this, SLOT(OnResourceEntered(const QString&)));
  connect(ui->pbCancel, SIGNAL(clicked()), this, SLOT(OnCancel()));
  connect(ui->pbUpload, SIGNAL(clicked()), this, SLOT(OnUpload()));

  // Initial Setup
  ui->pbUpload->setEnabled(false);
  ui->leResourceName->setVisible(false);
  ui->rbSelectFromTreeView->hide();
  ui->rbSelectResource->hide();
  ui->cbSelectResources->hide();

//  Temporarily disabled, only selection via treeview possible
//  if (availableResources.isEmpty())
//  {
//    ui->rbSelectFromTreeView->setChecked(true);
//    ui->rbSelectResource->setEnabled(false);
//  }
//  else
//  {
//    foreach (QString resourceName, availableResources)
//    {
//      ui->cbSelectResources->addItem(resourceName);
//    }
//    ui->cbSelectResources->addItem("Create new resource folder...");
//  }
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

void QmitkSelectXnatUploadDestinationDialog::SetXnatResourceFolderUrl(const QString &/*url*/)
{
//  m_XnatResourceFolderUrl = url;
}

ctkXnatResource *QmitkSelectXnatUploadDestinationDialog::GetUploadDestination()
{
  return m_SelectedResource;
}

void QmitkSelectXnatUploadDestinationDialog::OnResourceSelected(const QString& resource)
{
  if (resource.isEmpty())
    ui->pbUpload->setEnabled(false);

  if (resource != "Create new resource folder...")
  {
    ui->pbUpload->setEnabled(true);
    ui->leResourceName->hide();
  }
  else if (resource == "Create new resource folder...")
  {
    ui->pbUpload->setEnabled(false);
    ui->leResourceName->show();
  }
}

void QmitkSelectXnatUploadDestinationDialog::OnResourceEntered(const QString& resourceEntered)
{
  ui->pbUpload->setEnabled(!resourceEntered.isEmpty());
}

void QmitkSelectXnatUploadDestinationDialog::OnXnatNodeSelected(const QModelIndex& index)
{
  if (!index.isValid())
    return;

  ctkXnatObject* selectedObject = m_TreeModel->xnatObject(index);
  m_SelectedResource = dynamic_cast<ctkXnatResource*>(selectedObject);
  ui->pbUpload->setEnabled(m_SelectedResource != nullptr);
}

