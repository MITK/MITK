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


// Qmitk
#include "QmitkXnatEditor.h"
#include "QmitkXnatObjectEditorInput.h"
#include "QmitkXnatTreeBrowserView.h"

// Standard
#include <iostream>
#include <string>
#include <fstream>

// CTK XNAT Core
#include "ctkXnatLoginProfile.h"
#include "ctkXnatObject.h"
#include "ctkXnatProject.h"
#include "ctkXnatSubject.h"
#include "ctkXnatListModel.h"
#include "ctkXnatScanFolder.h"
#include "ctkXnatScanResource.h"

// CTK XNAT Widgets
#include "ctkXnatTreeModel.h"
#include "ctkXnatTreeItem.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryQModelIndexObject.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchPart.h>

// Qt
#include <QScopedPointer>
#include <QSharedPointer>
#include <QMessageBox>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QBrush>
#include <QListView>
#include <QRegExp>
#include <QModelIndex>

const std::string QmitkXnatEditor::EDITOR_ID = "org.mitk.editors.qmitkxnateditor";
const QString QmitkXnatEditor::DOWNLOAD_PATH = QString("C:/Users/knorr/Downloads/MITK_XNAT/");

QmitkXnatEditor::QmitkXnatEditor() :
  m_ListModel(new ctkXnatListModel()),
  m_SelectionListener(new berry::SelectionChangedAdapter<QmitkXnatEditor>(this, &QmitkXnatEditor::SelectionChanged))
{

}

QmitkXnatEditor::~QmitkXnatEditor()
{
  delete m_ListModel;

  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  s->RemoveSelectionListener(m_SelectionListener);
}

bool QmitkXnatEditor::IsDirty() const
{
  return false;
}

bool QmitkXnatEditor::IsSaveAsAllowed() const
{
  return false;
}

void QmitkXnatEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
   this->SetSite(site);
   this->SetInput(input);
}

void QmitkXnatEditor::DoSave()
{
}

void QmitkXnatEditor::DoSaveAs()
{
}

void QmitkXnatEditor::SetInput(berry::IEditorInput::Pointer input)
{
  SetInputWithNotify(input);
}
//
//void QmitkXnatEditor::AddPropertyListener(berry::IPropertyChangeListener::Pointer listener)
//{
//}
//
//void QmitkXnatEditor::CreatePartControl(void* parent)
//{
//}

const char* QmitkXnatEditor::GetClassNameA() const
{
  return QtEditorPart::GetClassNameA();
}

void QmitkXnatEditor::SetFocus()
{
}

void QmitkXnatEditor::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  m_Controls.treeView->setModel(m_ListModel);
  //UpdateList();

  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener);

  //connect( m_Controls.buttonHigherLevel, SIGNAL(clicked()), this, SLOT(OnHigherLevel()) );
  connect( m_Controls.treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(OnObjectActivated(const QModelIndex&)) );
  //connect( m_Controls.buttonDownloadResource, SIGNAL(clicked()), this, SLOT(DownloadResource()) );
  //connect( m_Controls.buttonDownloadFile, SIGNAL(clicked()), this, SLOT(DownloadFile()) );
  connect( m_Controls.buttonDataModel, SIGNAL(clicked()), this, SLOT(OnDataModelButtonClicked()) );
  connect( m_Controls.buttonProject, SIGNAL(clicked()), this, SLOT(OnProjectButtonClicked()) );
  connect( m_Controls.buttonSubject, SIGNAL(clicked()), this, SLOT(OnSubjectButtonClicked()) );
  connect( m_Controls.buttonExperiment, SIGNAL(clicked()), this, SLOT(OnExperimentButtonClicked()) );
  connect( m_Controls.buttonKindOfData, SIGNAL(clicked()), this, SLOT(OnKindOfDataButtonClicked()) );
  connect( m_Controls.buttonSession, SIGNAL(clicked()), this, SLOT(OnSessionButtonClicked()) );
  connect( m_Controls.buttonResource, SIGNAL(clicked()), this, SLOT(OnResourceButtonClicked()) );

  for(int i = 0; i < m_Controls.breadcrumbHorizontalLayout->count()-1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbHorizontalLayout->itemAt(i);
    child->widget()->setVisible(false);
  }
  for(int i = 0; i < m_Controls.breadcrumbDescriptionLayout->count()-1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbHorizontalLayout->itemAt(i);
    child->widget()->setVisible(false);
  }
}

void QmitkXnatEditor::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes )
{
}

/**
\brief Here the root object will be set and the view reset.
*/
void QmitkXnatEditor::UpdateList()
{
  ctkXnatObject* inputObject = GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject();
  if( inputObject == NULL )
    return;
  m_ListModel->setRootObject( inputObject );
  m_Controls.treeView->reset();

  // recursive method to check parents of the inputObject
  m_ParentCount = ParentChecker(inputObject);

  for(int i = 0; i < m_Controls.breadcrumbHorizontalLayout->count()-1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbHorizontalLayout->itemAt(i);
    child->widget()->setVisible(false);
  }
  for(int i = 0; i < m_Controls.breadcrumbDescriptionLayout->count()-1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbDescriptionLayout->itemAt(i);
    child->widget()->setVisible(false);
  }

  ctkXnatObject* parent = NULL;
  for(int i = m_ParentCount*2; i >= 0; i--)
  {
    m_Controls.breadcrumbDescriptionLayout->itemAt(i)->widget()->setVisible(true);
    QLayoutItem* child = m_Controls.breadcrumbHorizontalLayout->itemAt(i);
    child->widget()->setVisible(true);
    if(i>0)
    {
      m_Controls.breadcrumbHorizontalLayout->itemAt(i-1)->widget()->setVisible(true);
      m_Controls.breadcrumbDescriptionLayout->itemAt(i-1)->widget()->setVisible(true);
    }
    if(parent == NULL)
    {
      parent = inputObject;
    }
    QPushButton* breadcrumbButton = dynamic_cast<QPushButton*>(child->widget());
    breadcrumbButton->setText(parent->id());
    parent = parent->parent();
    i--;
  }
}

/**
\brief *****fetchs subjects from chosen project*****
*/
void QmitkXnatEditor::ProjectSelected(const QModelIndex& index)
{
  //m_ExperimentsModel->setRootObject(new ctkXnatProject);
  //m_ScansModel->setRootObject(new ctkXnatProject);
  //m_ResourceModel->setRootObject(new ctkXnatProject);
  //m_FileModel->setRootObject(new ctkXnatProject);
  //m_Controls.experimentTreeView->reset();
  //m_Controls.scanTreeView->reset();
  //m_Controls.resourceTreeView->reset();
  //m_Controls.fileTreeView->reset();
  //QVariant variant = m_ProjectsModel->data(index, Qt::UserRole);
  //if ( variant.isValid() )
  //{
  //  ctkXnatObject* project = variant.value<ctkXnatObject*>();
  //  project->fetch();
  //  m_SubjectsModel->setRootObject(project);
  //  m_Controls.subjectTreeView->reset();
  //}
}

/**
\brief *****fetchs experiments from chosen subject*****
*/
void QmitkXnatEditor::SubjectSelected(const QModelIndex& index)
{
  //m_ScansModel->setRootObject(new ctkXnatProject);
  //m_ResourceModel->setRootObject(new ctkXnatProject);
  //m_FileModel->setRootObject(new ctkXnatProject);
  //m_Controls.scanTreeView->reset();
  //m_Controls.resourceTreeView->reset();
  //m_Controls.fileTreeView->reset();
  //QVariant variant = m_SubjectsModel->data(index, Qt::UserRole);
  //if ( variant.isValid() )
  //{
  //  ctkXnatObject* subject = variant.value<ctkXnatObject*>();
  //  subject->fetch();
  //  m_ExperimentsModel->setRootObject(subject);
  //  m_Controls.experimentTreeView->reset();
  //}
}

/**
\brief *****fetchs scans from chosen experiment*****
*/
void QmitkXnatEditor::ExperimentSelected(const QModelIndex& index)
{
  //m_ResourceModel->setRootObject(new ctkXnatProject);
  //m_FileModel->setRootObject(new ctkXnatProject);
  //m_Controls.resourceTreeView->reset();
  //m_Controls.fileTreeView->reset();
  //QVariant variant = m_ExperimentsModel->data(index, Qt::UserRole);
  //if ( variant.isValid() )
  //{
  //  ctkXnatObject* experiment = variant.value<ctkXnatObject*>();
  //  experiment->fetch();
  //  if( !experiment->children().isEmpty() )
  //  {
  //    ctkXnatObject* scanfolder =  experiment->children().takeFirst();
  //    scanfolder->fetch();
  //    m_ScansModel->setRootObject(scanfolder);
  //    m_Controls.scanTreeView->reset();
  //  }
  //}
}

/**
\brief *****fetchs resource data from chosen experiment*****
*/
void QmitkXnatEditor::ScanSelected(const QModelIndex& index)
{
  //m_FileModel->setRootObject(new ctkXnatProject);
  //m_Controls.fileTreeView->reset();
  //QVariant variant = m_ScansModel->data(index, Qt::UserRole);
  //if ( variant.isValid() )
  //{
  //  ctkXnatObject* scan = variant.value<ctkXnatObject*>();
  //  scan->fetch();
  //  m_ResourceModel->setRootObject(scan);
  //  m_Controls.resourceTreeView->reset();
  //}
}

/**
\brief *****fetchs files data from chosen resource*****
*/
void QmitkXnatEditor::ResourceSelected(const QModelIndex& index)
{
  //QVariant variant = m_ResourceModel->data(index, Qt::UserRole);
  //if ( variant.isValid() )
  //{
  //  ctkXnatObject* resource = variant.value<ctkXnatObject*>();
  //  resource->fetch();
  //  m_FileModel->setRootObject(resource);
  //  m_Controls.fileTreeView->reset();
  //}
}
/*****************************DOWNLOADS START**************************************
void QmitkXnatEditor::DownloadResource()
{
  if (!m_Controls.resourceTreeView->selectionModel()->hasSelection())
    return;
  const QModelIndex index = m_Controls.resourceTreeView->selectionModel()->currentIndex();
  QVariant variant = m_ResourceModel->data(index, Qt::UserRole);
  if ( variant.isValid() )
  {
    ctkXnatObject* resource = variant.value<ctkXnatObject*>();
    MITK_INFO << "Download started ...";
    MITK_INFO << "...";
    QString resourcePath = DOWNLOAD_PATH + resource->id() + ".zip";
    resource->download(resourcePath);
    MITK_INFO << "Download finished!";
  }
}

void QmitkXnatEditor::DownloadFile()
{
  if (!m_Controls.fileTreeView->selectionModel()->hasSelection())
    return;
  const QModelIndex index = m_Controls.fileTreeView->selectionModel()->currentIndex();
  QVariant variant = m_FileModel->data(index, Qt::UserRole);
  if ( variant.isValid() )
  {
    ctkXnatObject* file = variant.value<ctkXnatObject*>();
    MITK_INFO << "Download started ...";
    MITK_INFO << "...";
    QString filePath = DOWNLOAD_PATH + file->id();

    file->download(filePath);
    MITK_INFO << "Download finished!";
  }
}

bool QmitkXnatEditor::DownloadExists(QString filename)
{
  std::ifstream fileTest(filename.toStdString().c_str());
  return (fileTest ? true : false);
}
*********************************DOWNLOADS ENDE************************************/

void QmitkXnatEditor::SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart,
                               berry::ISelection::ConstPointer selection)
{
  // check for null selection
  if (selection.IsNull())
  {
    return;
  }
  // exclude own selection events and check whether this kind of selection can be handled
  if (sourcepart != this &&
      selection.Cast<const berry::IStructuredSelection>())
  {
    berry::IStructuredSelection::ConstPointer currentSelection = selection.Cast<const berry::IStructuredSelection>();
    // iterates over the selection
    for (berry::IStructuredSelection::iterator itr = currentSelection->Begin();
         itr != currentSelection->End(); ++itr)
    {
      if (berry::SmartPointer<berry::QModelIndexObject> objectPointer = itr->Cast<berry::QModelIndexObject>())
      {
        // get object of selected ListWidgetElement
        ctkXnatObject* object = objectPointer->GetQModelIndex().data(Qt::EditRole).value<ctkXnatObject*>();
        MITK_INFO << objectPointer->GetQModelIndex().data().toString().toStdString() << " is selected!";
        QmitkXnatObjectEditorInput::Pointer oPtr = QmitkXnatObjectEditorInput::New( object );
        berry::IEditorInput::Pointer editorInput( oPtr );
        if ( !(editorInput == this->GetEditorInput()) )
          SetInput(editorInput);

        UpdateList();
      }
    }
  }
}

void QmitkXnatEditor::ToHigherLevel()
{
  ctkXnatObject* parent = GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->parent();
  if( parent ==  NULL)
  {
    return;
  }
  QmitkXnatObjectEditorInput::Pointer oPtr = QmitkXnatObjectEditorInput::New( parent );
  berry::IEditorInput::Pointer editorInput( oPtr );
  SetInput(editorInput);
  UpdateList();
}

void QmitkXnatEditor::OnObjectActivated(const QModelIndex &index)
{
  if (!index.isValid()) return;

  ctkXnatObject* child = GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->children().takeAt(index.row());
  if( child ==  NULL)
  {
    return;
  }
  QmitkXnatObjectEditorInput::Pointer oPtr = QmitkXnatObjectEditorInput::New( child );
  berry::IEditorInput::Pointer editorInput( oPtr );
  SetInput(editorInput);

  if ( !this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->isFetched() )
  {
    this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->fetch();
  }
  UpdateList();
}

int QmitkXnatEditor::ParentChecker(ctkXnatObject* child)
{
  int sum;
  if( child->parent() == NULL )
  {
    return 0;
  }
  else
  {
    sum = 1 + ParentChecker(child->parent());
  }
  return sum;
}

void QmitkXnatEditor::OnDataModelButtonClicked()
{
  for(int i = m_ParentCount; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnProjectButtonClicked()
{
  for(int i = m_ParentCount-1; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnSubjectButtonClicked()
{
  for(int i = m_ParentCount-2; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnExperimentButtonClicked()
{
  for(int i = m_ParentCount-3; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnKindOfDataButtonClicked()
{
  for(int i = m_ParentCount-4; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnSessionButtonClicked()
{
  for(int i = m_ParentCount-5; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnResourceButtonClicked()
{
  for(int i = m_ParentCount-6; i > 0; i--)
  {
    ToHigherLevel();
  }
}
