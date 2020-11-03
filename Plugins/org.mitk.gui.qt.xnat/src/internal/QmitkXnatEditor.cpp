/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkXnatEditor.h"

// Qmitk
#include "QmitkXnatObjectEditorInput.h"
#include "org_mitk_gui_qt_xnatinterface_Activator.h"

// CTK XNAT Core
#include "ctkXnatObject.h"
#include "ctkXnatDataModel.h"
#include "ctkXnatFile.h"
#include "ctkXnatResource.h"
#include "ctkXnatScan.h"
#include "ctkXnatScanFolder.h"
#include "ctkXnatAssessor.h"
#include "ctkXnatAssessorFolder.h"
#include "ctkXnatReconstruction.h"
#include "ctkXnatReconstructionFolder.h"

// CTK XNAT Widgets
#include "ctkXnatListModel.h"

// Blueberry
#include <berryQModelIndexObject.h>
#include <berryIWorkbenchPage.h>
#include <berryPlatform.h>

// Qt
#include <QListView>
#include <QRegExp>
#include <QModelIndex>
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>

// MITK
#include <mitkDataStorage.h>
#include <QmitkIOUtil.h>

// Poco
#include <Poco/Zip/Decompress.h>

const QString QmitkXnatEditor::EDITOR_ID = "org.mitk.editors.xnat.browser";

QmitkXnatEditor::QmitkXnatEditor() :
m_DownloadPath(berry::Platform::GetPreferencesService()->
GetSystemPreferences()->Node("/XnatConnection")->Get("Download Path", "")),
m_ListModel(new ctkXnatListModel()),
m_Session(0),
m_DataStorageServiceTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetContext()),
m_SelectionListener(new berry::SelectionChangedAdapter<QmitkXnatEditor>(this, &QmitkXnatEditor::SelectionChanged))
{
  m_DataStorageServiceTracker.open();
  if (m_DownloadPath.isEmpty())
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
  s->RemoveSelectionListener(m_SelectionListener.data());
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
  if (oPtr.IsNotNull())
  {
    berry::QtEditorPart::SetInput(oPtr);
    this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->fetch();
  }
}

void QmitkXnatEditor::SetFocus()
{
}

void QmitkXnatEditor::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  m_Controls.buttonDownload->setEnabled(false);
  m_Controls.labelDownload->setText("Select a xnat file, resource, scan, or scan folder to download...");

  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener.data());

  connect(m_Controls.treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(OnObjectActivated(const QModelIndex&)));

  connect(m_Controls.buttonDownload, SIGNAL(clicked()), this, SLOT(DownloadResource()));
  connect(m_Controls.buttonDataModel, SIGNAL(clicked()), this, SLOT(OnDataModelButtonClicked()));
  connect(m_Controls.buttonProject, SIGNAL(clicked()), this, SLOT(OnProjectButtonClicked()));
  connect(m_Controls.buttonSubject, SIGNAL(clicked()), this, SLOT(OnSubjectButtonClicked()));
  connect(m_Controls.buttonExperiment, SIGNAL(clicked()), this, SLOT(OnExperimentButtonClicked()));
  connect(m_Controls.buttonKindOfData, SIGNAL(clicked()), this, SLOT(OnKindOfDataButtonClicked()));
  connect(m_Controls.buttonSession, SIGNAL(clicked()), this, SLOT(OnSessionButtonClicked()));
  connect(m_Controls.buttonResource, SIGNAL(clicked()), this, SLOT(OnResourceButtonClicked()));
  connect(m_Controls.treeView, SIGNAL(clicked(const QModelIndex&)), SLOT(itemSelected(const QModelIndex&)));

  m_Tracker = new mitk::XnatSessionTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext());

  connect(m_Tracker, SIGNAL(AboutToBeClosed(ctkXnatSession*)), this, SLOT(CleanListModel(ctkXnatSession*)));
  connect(m_Tracker, SIGNAL(Opened(ctkXnatSession*)), this, SLOT(UpdateSession(ctkXnatSession*)));

  m_Tracker->Open();

  // Makes the breadcrumb feature invisible
  for (int i = 0; i < m_Controls.breadcrumbHorizontalLayout->count() - 1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbHorizontalLayout->itemAt(i);
    child->widget()->setVisible(false);
  }
  for (int i = 0; i < m_Controls.breadcrumbDescriptionLayout->count() - 1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbDescriptionLayout->itemAt(i);
    child->widget()->setVisible(false);
  }
  QmitkXnatObjectEditorInput::Pointer oPtr = GetEditorInput().Cast<QmitkXnatObjectEditorInput>();
  if (oPtr.IsNotNull())
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
    catch (std::invalid_argument)
    {
      session = 0;
    }
    UpdateSession(session);
  }
}

void QmitkXnatEditor::UpdateList()
{
  QmitkXnatObjectEditorInput::Pointer xoPtr(GetEditorInput().Cast<QmitkXnatObjectEditorInput>());
  if (xoPtr.IsNull())
    return;
  ctkXnatObject* inputObject = xoPtr->GetXnatObject();
  if (inputObject == nullptr)
    return;

  m_Controls.treeView->setModel(m_ListModel);
  m_ListModel->setRootObject(inputObject);
  m_Controls.treeView->reset();

  // recursive method to check parents of the inputObject
  m_ParentCount = ParentChecker(inputObject);

  // breadcrumb labels
  for (int i = 0; i < m_Controls.breadcrumbHorizontalLayout->count() - 1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbHorizontalLayout->itemAt(i);
    child->widget()->setVisible(false);
  }
  for (int i = 0; i < m_Controls.breadcrumbDescriptionLayout->count() - 1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbDescriptionLayout->itemAt(i);
    child->widget()->setVisible(false);
  }

  ctkXnatObject* parent = nullptr;
  for (int i = m_ParentCount * 2; i >= 0; i--)
  {
    if (i > 12)
      break;
    m_Controls.breadcrumbDescriptionLayout->itemAt(i)->widget()->setVisible(true);
    QLayoutItem* child = m_Controls.breadcrumbHorizontalLayout->itemAt(i);
    child->widget()->setVisible(true);
    if (i > 0)
    {
      m_Controls.breadcrumbHorizontalLayout->itemAt(i - 1)->widget()->setVisible(true);
      m_Controls.breadcrumbDescriptionLayout->itemAt(i - 1)->widget()->setVisible(true);
    }
    if (parent == nullptr)
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
  m_Controls.buttonDownload->setEnabled(false);
  m_Controls.labelDownload->setVisible(true);
}

void QmitkXnatEditor::SelectionChanged(const berry::IWorkbenchPart::Pointer& sourcepart,
                                       const berry::ISelection::ConstPointer& selection)
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
        if (dynamic_cast<ctkXnatFile*>(object) == nullptr)
        {
          QmitkXnatObjectEditorInput::Pointer oPtr(new QmitkXnatObjectEditorInput(object));
          berry::IEditorInput::Pointer editorInput(oPtr);
          if (!(editorInput == this->GetEditorInput()))
            this->SetInput(editorInput);

          UpdateList();
        }
      }
    }
  }
}

void QmitkXnatEditor::DownloadResource()
{
  if (!m_Controls.listView->selectionModel()->hasSelection())
    return;

  const QModelIndex index = m_Controls.listView->selectionModel()->currentIndex();
  QVariant variant = m_ListModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    ctkXnatObject* resource = variant.value<ctkXnatObject*>();
    if (dynamic_cast<ctkXnatFile*>(resource) == nullptr)
    {
      MITK_INFO << "Download started ...";
      MITK_INFO << "...";
      QString resourceName = m_ListModel->data(index, Qt::DisplayRole).toString();
      QString resourcePath = m_DownloadPath + resourceName + ".zip";
      resource->download(resourcePath);

      // Testing if the path exists
      QDir downDir(m_DownloadPath);
      if (downDir.exists(resourceName + ".zip"))
      {
        MITK_INFO << "Download of " << resourceName.toStdString() << ".zip was completed!";
      }
      else
      {
        MITK_INFO << "Download of " << resourceName.toStdString() << ".zip failed!";
      }
    }
    else
    {
      InternalFileDownload(index);
    }
  }
}

void QmitkXnatEditor::DownloadFile()
{
  if (!m_Controls.listView->selectionModel()->hasSelection())
    return;
  const QModelIndex index = m_Controls.listView->selectionModel()->currentIndex();
  InternalFileDownload(index);
}

void QmitkXnatEditor::ToHigherLevel()
{
  ctkXnatObject* parent = GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->parent();
  if (parent == nullptr)
  {
    return;
  }
  QmitkXnatObjectEditorInput::Pointer oPtr(new QmitkXnatObjectEditorInput(parent));
  berry::IEditorInput::Pointer editorInput(oPtr);
  this->SetInput(editorInput);
  UpdateList();
}

void QmitkXnatEditor::OnObjectActivated(const QModelIndex &index)
{
  if (!index.isValid()) return;

  ctkXnatObject* child = GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->children().at(index.row());
  if (child != nullptr)
  {
    ctkXnatFile* file = dynamic_cast<ctkXnatFile*>(child);
    if (file != nullptr)
    {
      // Download file and put into datamanager
      InternalFileDownload(index);
      mitk::IDataStorageService* dsService = m_DataStorageServiceTracker.getService();
      if (dsService != nullptr)
      {
        QString name = file->property("Name");
        QString filePath = m_DownloadPath + name;

        if (file->property("collection") == "DICOM")
        {
          QDirIterator it(m_DownloadPath, QStringList() << name, QDir::Files, QDirIterator::Subdirectories);
          while (it.hasNext()) {
            it.next();
            filePath = it.filePath();
          }
        }

        mitk::IDataStorageService* dsService = m_DataStorageServiceTracker.getService();
        mitk::DataStorage::Pointer dataStorage = dsService->GetDataStorage()->GetDataStorage();
        QStringList list;
        list << filePath;
        try
        {
          QmitkIOUtil::Load(list, *dataStorage);
        }
        catch (const mitk::Exception& e)
        {
          MITK_INFO << e;
          return;
        }
        mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(
          dsService->GetDataStorage()->GetDataStorage());
      }
    }
    else
    {
      // Updates the root item
      QmitkXnatObjectEditorInput::Pointer oPtr(new QmitkXnatObjectEditorInput(child));
      berry::IEditorInput::Pointer editorInput(oPtr);
      this->SetInput(editorInput);

      this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->fetch();

      UpdateList();
    }
  }
}

void QmitkXnatEditor::InternalFileDownload(const QModelIndex& index)
{
  QVariant variant = m_ListModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    ctkXnatFile* file = dynamic_cast<ctkXnatFile*>(variant.value<ctkXnatObject*>());
    if (file != nullptr)
    {
      // Testing if the file exists
      QDir downDir(m_DownloadPath);
      if (downDir.exists(file->property("Name")))
      {
        MITK_INFO << "File exists already!";
        return;
      }

      if (file->property("collection") == QString("DICOM"))
      {
        ctkXnatObject* parent = file->parent();

        QString filePath = m_DownloadPath + parent->property("label") + ".zip";
        parent->download(filePath);

        std::ifstream in(filePath.toStdString().c_str(), std::ios::binary);
        poco_assert(in);

        // decompress to XNAT_DOWNLOAD dir
        Poco::Zip::Decompress dec(in, Poco::Path(m_DownloadPath.toStdString()));
        dec.decompressAllFiles();

        in.close();
        QFile::remove(filePath);
      }
      else
      {
        MITK_INFO << "Download started ...";
        MITK_INFO << "...";
        QString name = file->property("Name");
        QString filePath = m_DownloadPath + name;
        file->download(filePath);

        // Testing if the file exists
        QDir downDir(m_DownloadPath);
        if (downDir.exists(name))
        {
          MITK_INFO << "Download of " << file->name().toStdString() << " was completed!";
        }
        else
        {
          MITK_INFO << "Download of " << file->name().toStdString() << " failed!";
        }
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
  if (child->parent() == nullptr)
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
  for (int i = m_ParentCount; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnProjectButtonClicked()
{
  for (int i = m_ParentCount - 1; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnSubjectButtonClicked()
{
  for (int i = m_ParentCount - 2; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnExperimentButtonClicked()
{
  for (int i = m_ParentCount - 3; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnKindOfDataButtonClicked()
{
  for (int i = m_ParentCount - 4; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnSessionButtonClicked()
{
  for (int i = m_ParentCount - 5; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnResourceButtonClicked()
{
  for (int i = m_ParentCount - 6; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::UpdateSession(ctkXnatSession* session)
{
  GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemoveSelectionListener(m_SelectionListener.data());

  if (session != 0 && session->isOpen())
  {
    m_Controls.labelInfo->setText("Current Position:");
    m_Controls.labelInfo->setStyleSheet("QLabel { color: black; }");

    // Fill model and show in the GUI
    QmitkXnatObjectEditorInput::Pointer xoPtr(new QmitkXnatObjectEditorInput(session->dataModel()));
    berry::IEditorInput::Pointer editorInput(xoPtr);
    this->SetInput(editorInput);
    this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->fetch();
    UpdateList();
  }
  else
  {
    m_Controls.labelInfo->setText("Please check the Preferences of the XNAT Connection.\nMaybe they are not ok.");
    m_Controls.labelInfo->setStyleSheet("QLabel { color: red; }");
  }

  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener.data());
}

void QmitkXnatEditor::CleanListModel(ctkXnatSession* session)
{
  if (session != 0)
  {
    m_Controls.listView->setModel(0);
    m_ListModel->setRootObject(0);
    m_Controls.treeView->reset();
  }
}

void QmitkXnatEditor::itemSelected(const QModelIndex &index)
{
  ctkXnatObject* xnatObject = m_ListModel->data(index, Qt::UserRole).value<ctkXnatObject*>();
  bool downloadable = false;
  downloadable |= dynamic_cast<ctkXnatFile*>(xnatObject) != nullptr;
  downloadable |= dynamic_cast<ctkXnatScan*>(xnatObject) != nullptr;
  downloadable |= dynamic_cast<ctkXnatScanFolder*>(xnatObject) != nullptr;
  downloadable |= dynamic_cast<ctkXnatAssessor*>(xnatObject) != nullptr;
  downloadable |= dynamic_cast<ctkXnatAssessorFolder*>(xnatObject) != nullptr;
  downloadable |= dynamic_cast<ctkXnatResource*>(xnatObject) != nullptr;
  downloadable |= dynamic_cast<ctkXnatReconstruction*>(xnatObject) != nullptr;
  downloadable |= dynamic_cast<ctkXnatReconstructionFolder*>(xnatObject) != nullptr;
  m_Controls.buttonDownload->setEnabled(downloadable);
  m_Controls.labelDownload->setVisible(!downloadable);
}
