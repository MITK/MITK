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

#include "QmitkXnatEditor.h"

// Qmitk
#include "QmitkXnatObjectEditorInput.h"
#include "org_mitk_gui_qt_xnatinterface_Activator.h"

// CTK XNAT Core
#include "ctkXnatObject.h"
#include "ctkXnatDataModel.h"
#include "ctkXnatScanFolder.h"
#include "ctkXnatFile.h"

// CTK XNAT Widgets
#include "ctkXnatListModel.h"

// Blueberry
#include <berryQModelIndexObject.h>
#include <berryIWorkbenchPage.h>

// Qt
#include <QListView>
#include <QRegExp>
#include <QModelIndex>
#include <QDir>
#include <QMessageBox>

// MITK
#include <mitkDataStorage.h>
#include <mitkIOUtil.h>

const std::string QmitkXnatEditor::EDITOR_ID = "org.mitk.editors.xnat.browser";

QmitkXnatEditor::QmitkXnatEditor() :
  m_DataStorageServiceTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetContext()),
  m_Session(0),
  m_ListModel(new ctkXnatListModel()),
  m_SelectionListener(new berry::SelectionChangedAdapter<QmitkXnatEditor>(this, &QmitkXnatEditor::SelectionChanged)),
  m_DownloadPath(berry::Platform::GetServiceRegistry().
    GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID)->
    GetSystemPreferences()->Node("/XnatConnection")->Get("Download Path", "").c_str())
{
  m_DataStorageServiceTracker.open();
  if(m_DownloadPath.isEmpty())
  {
    QString xnatFolder = "XNAT_DOWNLOADS";
    QDir dir(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetContext()->getDataFile("").absoluteFilePath());
    dir.mkdir(xnatFolder);
    dir.setPath(dir.path() + "/" + xnatFolder);
    m_DownloadPath = dir.path() + "/";
  }
}

QmitkXnatEditor::~QmitkXnatEditor()
{
  delete m_ListModel;
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  s->RemoveSelectionListener(m_SelectionListener);
  m_DataStorageServiceTracker.close();
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
  berry::QtEditorPart::SetInput(input);
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
  QmitkXnatObjectEditorInput::Pointer oPtr = input.Cast<QmitkXnatObjectEditorInput>();
  if(oPtr.IsNotNull())
  {
    berry::QtEditorPart::SetInput(oPtr);
    this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->fetch();
  }
}

void QmitkXnatEditor::SetFocus()
{
}

void QmitkXnatEditor::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener);

  connect( m_Controls.treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(OnObjectActivated(const QModelIndex&)) );

  connect( m_Controls.buttonDownloadResource, SIGNAL(clicked()), this, SLOT(DownloadResource()) );
  connect( m_Controls.buttonDownloadFile, SIGNAL(clicked()), this, SLOT(DownloadFile()) );
  connect( m_Controls.buttonDataModel, SIGNAL(clicked()), this, SLOT(OnDataModelButtonClicked()) );
  connect( m_Controls.buttonProject, SIGNAL(clicked()), this, SLOT(OnProjectButtonClicked()) );
  connect( m_Controls.buttonSubject, SIGNAL(clicked()), this, SLOT(OnSubjectButtonClicked()) );
  connect( m_Controls.buttonExperiment, SIGNAL(clicked()), this, SLOT(OnExperimentButtonClicked()) );
  connect( m_Controls.buttonKindOfData, SIGNAL(clicked()), this, SLOT(OnKindOfDataButtonClicked()) );
  connect( m_Controls.buttonSession, SIGNAL(clicked()), this, SLOT(OnSessionButtonClicked()) );
  connect( m_Controls.buttonResource, SIGNAL(clicked()), this, SLOT(OnResourceButtonClicked()) );

  m_Tracker = new mitk::XnatSessionTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext());

  connect( m_Tracker, SIGNAL(AboutToBeClosed(ctkXnatSession*)), this, SLOT(CleanListModel(ctkXnatSession*)) );
  connect( m_Tracker, SIGNAL(Opened(ctkXnatSession*)), this, SLOT(UpdateSession(ctkXnatSession*)) );

  m_Tracker->Open();

  // Makes the breadcrumb feature invisible
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
  QmitkXnatObjectEditorInput::Pointer oPtr = GetEditorInput().Cast<QmitkXnatObjectEditorInput>();
  if(oPtr.IsNotNull())
  {
    UpdateList();
  }
  else
  {
    ctkXnatSession* session;
    try
    {
      session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
        mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
    }
    catch(std::invalid_argument)
    {
      session = 0;
    }
    UpdateSession(session);
  }
}

void QmitkXnatEditor::UpdateList()
{
  QmitkXnatObjectEditorInput::Pointer xoPtr(GetEditorInput().Cast<QmitkXnatObjectEditorInput>());
  if( xoPtr.IsNull() )
    return;
  ctkXnatObject* inputObject = xoPtr->GetXnatObject();
  if( inputObject == NULL )
    return;

  m_Controls.treeView->setModel(m_ListModel);
  m_ListModel->setRootObject( inputObject );
  m_Controls.treeView->reset();

  // recursive method to check parents of the inputObject
  m_ParentCount = ParentChecker(inputObject);

  // breadcrumb labels
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
    if(i > 12)
      break;
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
    // create breadcrumb button
    QPushButton* breadcrumbButton = dynamic_cast<QPushButton*>(child->widget());
    breadcrumbButton->setText(parent->id());
    parent = parent->parent();
    i--;
  }
  m_Controls.buttonDataModel->setText("root");
}

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
        ctkXnatObject* object = objectPointer->GetQModelIndex().data(Qt::UserRole).value<ctkXnatObject*>();

        // if a file is selected, don't change the input and list view
        if ( dynamic_cast<ctkXnatFile*>(object) == NULL )
        {
          QmitkXnatObjectEditorInput::Pointer oPtr = QmitkXnatObjectEditorInput::New( object );
          berry::IEditorInput::Pointer editorInput( oPtr );
          if ( !(editorInput == this->GetEditorInput()) )
            this->SetInput(editorInput);

          UpdateList();
        }
      }
    }
  }
}

void QmitkXnatEditor::DownloadResource()
{
  if (!m_Controls.treeView->selectionModel()->hasSelection())
    return;

  const QModelIndex index = m_Controls.treeView->selectionModel()->currentIndex();
  QVariant variant = m_ListModel->data(index, Qt::UserRole);
  if ( variant.isValid() )
  {
    ctkXnatScanFolder* resource = dynamic_cast<ctkXnatScanFolder*>(variant.value<ctkXnatObject*>());
    if (resource != NULL)
    {
      MITK_INFO << "Download started ...";
      MITK_INFO << "...";
      QString resourcePath = m_DownloadPath + resource->id() + ".zip";
      resource->download(resourcePath);

      // Testing if the path exists
      QDir downDir(m_DownloadPath);
      if( downDir.exists(resource->id() + ".zip") )
      {
        MITK_INFO << "Download of " << resource->id().toStdString() << ".zip was completed!";
      }
      else
      {
        MITK_INFO << "Download of " << resource->id().toStdString() << ".zip failed!";
      }
    }
    else
    {
      MITK_INFO << "Selection was not a resource folder!";
    }
  }
}

void QmitkXnatEditor::DownloadFile()
{
  if (!m_Controls.treeView->selectionModel()->hasSelection())
    return;
  const QModelIndex index = m_Controls.treeView->selectionModel()->currentIndex();
  InternalFileDownload(index);
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
  this->SetInput(editorInput);
  UpdateList();
}

void QmitkXnatEditor::OnObjectActivated(const QModelIndex &index)
{
  if (!index.isValid()) return;

  ctkXnatObject* child = GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->children().at(index.row());
  if( child != NULL )
  {
    ctkXnatFile* file = dynamic_cast<ctkXnatFile*>(child);
    if( file != NULL )
    {
      // Download file and put into datamanager
      InternalFileDownload(index);
      mitk::IDataStorageService* dsService = m_DataStorageServiceTracker.getService();
      if(dsService != NULL)
      {
        mitk::DataNode::Pointer node = mitk::IOUtil::LoadDataNode((m_DownloadPath + file->id()).toStdString());
        if ( ( node.IsNotNull() ) && ( node->GetData() != NULL ) )
        {
          dsService->GetDataStorage()->GetDataStorage()->Add(node);
          mitk::BaseData::Pointer basedata = node->GetData();
          mitk::RenderingManager::GetInstance()->InitializeViews(
            basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        }
      }
    }
    else
    {
      // Updates the root item
      QmitkXnatObjectEditorInput::Pointer oPtr = QmitkXnatObjectEditorInput::New( child );
      berry::IEditorInput::Pointer editorInput( oPtr );
      this->SetInput(editorInput);

      this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->fetch();

      UpdateList();
    }
  }
}

void QmitkXnatEditor::InternalFileDownload(const QModelIndex& index)
{
  QVariant variant = m_ListModel->data(index, Qt::UserRole);
  if ( variant.isValid() )
  {
    ctkXnatFile* file = dynamic_cast<ctkXnatFile*>(variant.value<ctkXnatObject*>());
    if (file != NULL)
    {
      MITK_INFO << "Download started ...";
      MITK_INFO << "...";
      QString filePath = m_DownloadPath + file->id();
      file->download(filePath);

      // Testing if the file exists
      QDir downDir(m_DownloadPath);
      if( downDir.exists(file->id()) )
      {
        MITK_INFO << "Download of " << file->id().toStdString() << " was completed!";
      }
      else
      {
        MITK_INFO << "Download of " << file->id().toStdString() << " failed!";
      }
    }
    else
    {
      MITK_INFO << "Selection was not a file!";
    }
  }
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

void QmitkXnatEditor::UpdateSession(ctkXnatSession* session)
{
  GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemoveSelectionListener(m_SelectionListener);

  if(session != 0 && session->isOpen())
  {
    m_Controls.labelInfo->setText("Current Position:");
    m_Controls.labelInfo->setStyleSheet("QLabel { color: black; }");
    m_Controls.buttonDownloadFile->setEnabled(true);
    m_Controls.buttonDownloadResource->setEnabled(true);

    // Fill model and show in the GUI
    QmitkXnatObjectEditorInput::Pointer xoPtr = QmitkXnatObjectEditorInput::New( session->dataModel() );
    berry::IEditorInput::Pointer editorInput( xoPtr );
    this->SetInput(editorInput);
    this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->fetch();
    UpdateList();
  }
  else
  {
    m_Controls.labelInfo->setText("Please check the Preferences of the XNAT Connection.\nMaybe they are not ok.");
    m_Controls.labelInfo->setStyleSheet("QLabel { color: red; }");
    m_Controls.buttonDownloadFile->setEnabled(false);
    m_Controls.buttonDownloadResource->setEnabled(false);
  }

  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener);
}

void QmitkXnatEditor::CleanListModel(ctkXnatSession* session)
{
  if(session != 0)
  {
    m_Controls.treeView->setModel(0);
    m_ListModel->setRootObject(0);
    m_Controls.treeView->reset();
    m_Controls.buttonDownloadFile->setEnabled(false);
    m_Controls.buttonDownloadResource->setEnabled(false);
  }
}
