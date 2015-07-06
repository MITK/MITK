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

#include "QmitkXnatTreeBrowserView.h"

// Qmitk
#include "QmitkXnatObjectEditorInput.h"
#include "QmitkXnatEditor.h"
#include "org_mitk_gui_qt_xnatinterface_Activator.h"

// Blueberry
#include <berryIWorkbenchPage.h>
#include <berryPlatform.h>

// CTK XNAT Core
#include <ctkXnatAssessor.h>
#include <ctkXnatAssessorFolder.h>
#include <ctkXnatException.h>
#include <ctkXnatExperiment.h>
#include "ctkXnatFile.h"
#include <ctkXnatProject.h>
#include <ctkXnatReconstruction.h>
#include <ctkXnatReconstructionFolder.h>
#include <ctkXnatResource.h>
#include <ctkXnatResourceFolder.h>
#include <ctkXnatScan.h>
#include <ctkXnatScanFolder.h>
#include <ctkXnatSubject.h>

// Qt
#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QDirIterator>
#include <QMessageBox>

// MITK
#include <mitkDataStorage.h>
#include <mitkProgressBar.h>
#include <QmitkIOUtil.h>

// Poco
#include <Poco/Zip/Decompress.h>

const std::string QmitkXnatTreeBrowserView::VIEW_ID = "org.mitk.views.xnat.treebrowser";

QmitkXnatTreeBrowserView::QmitkXnatTreeBrowserView() :
m_DataStorageServiceTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetContext()),
m_TreeModel(new QmitkXnatTreeModel()),
m_Tracker(0),
m_DownloadPath(berry::Platform::GetPreferencesService()->GetSystemPreferences()->Node("/XnatConnection")->Get("Download Path", ""))
{
  m_DataStorageServiceTracker.open();

  // Set DownloadPath
  if (m_DownloadPath.isEmpty())
  {
    QString xnatFolder = "XNAT_DOWNLOADS";
    QDir dir(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetContext()->getDataFile("").absoluteFilePath());
    dir.mkdir(xnatFolder);
    dir.setPath(dir.path() + "/" + xnatFolder);
    m_DownloadPath = dir.path() + "/";
  }
}

QmitkXnatTreeBrowserView::~QmitkXnatTreeBrowserView()
{
  delete m_TreeModel;
  delete m_Tracker;
}

void QmitkXnatTreeBrowserView::SetFocus()
{
}

void QmitkXnatTreeBrowserView::CreateQtPartControl(QWidget *parent)
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  m_Controls.treeView->setModel(m_TreeModel);
  m_Controls.treeView->header()->hide();
  m_Controls.treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.treeView->setAcceptDrops(true);
  m_Controls.treeView->setDropIndicatorShown(true);
  m_Controls.labelError->setText("Please use the 'Connect' button in the Preferences.");
  m_Controls.labelError->setStyleSheet("QLabel { color: red; }");

  m_SelectionProvider = new berry::QtSelectionProvider();
  this->SetSelectionProvider();
  m_Controls.treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.treeView->setContextMenuPolicy(Qt::CustomContextMenu);

  m_Tracker = new mitk::XnatSessionTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext());

  m_ContextMenu = new QMenu(m_Controls.treeView);

  connect(m_Controls.treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
    this, SLOT(OnContextMenuRequested(const QPoint&)));
  connect(m_Tracker, SIGNAL(AboutToBeClosed(ctkXnatSession*)), this, SLOT(CleanTreeModel(ctkXnatSession*)));
  connect(m_Tracker, SIGNAL(Opened(ctkXnatSession*)), this, SLOT(UpdateSession(ctkXnatSession*)));

  m_Tracker->Open();

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

  if (session != 0)
  {
    m_Controls.labelError->setVisible(false);
  }
  else
  {
    m_Controls.labelError->setVisible(true);
  }

  connect(m_Controls.treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(OnActivatedNode(const QModelIndex&)));
  connect(m_TreeModel, SIGNAL(ResourceDropped(const QList<mitk::DataNode*>&, ctkXnatObject*)), this, SLOT(OnUploadResource(const QList<mitk::DataNode*>&, ctkXnatObject*)));
}

void QmitkXnatTreeBrowserView::OnActivatedNode(const QModelIndex& index)
{
  if (!index.isValid()) return;

  berry::IWorkbenchPage::Pointer page = GetSite()->GetPage();
  QmitkXnatObjectEditorInput::Pointer oPtr(new QmitkXnatObjectEditorInput(index.data(Qt::UserRole).value<ctkXnatObject*>()));
  berry::IEditorInput::Pointer editorInput(oPtr);
  berry::IEditorPart::Pointer reuseEditor = page->FindEditor(editorInput);

  if (reuseEditor)
  {
    // Just set it activ
    page->Activate(reuseEditor);
  }
  else
  {
    QList<berry::IEditorReference::Pointer> editors =
      page->FindEditors(berry::IEditorInput::Pointer(0), QmitkXnatEditor::EDITOR_ID, berry::IWorkbenchPage::MATCH_ID);

    if (editors.isEmpty())
    {
      ctkXnatFile* file = dynamic_cast<ctkXnatFile*>(oPtr->GetXnatObject());
      if (file != NULL)
      {
        // If the selected node is a file, so show it in MITK
        InternalFileDownload(index, true);
      }
      else
      {
        // No XnatEditor is currently open, create a new one
        page->OpenEditor(editorInput, QmitkXnatEditor::EDITOR_ID);
      }
    }
    else
    {
      // Reuse an existing editor
      reuseEditor = editors.front()->GetEditor(true);
      page->ReuseEditor(reuseEditor.Cast<berry::IReusableEditor>(), editorInput);
      page->Activate(reuseEditor);
    }
  }
}

void QmitkXnatTreeBrowserView::SetSelectionProvider()
{
  GetSite()->SetSelectionProvider(m_SelectionProvider);
}

void QmitkXnatTreeBrowserView::UpdateSession(ctkXnatSession* session)
{
  if (session != 0 && session->isOpen())
  {
    m_Controls.labelError->setVisible(false);
    // Fill model and show in the GUI
    m_TreeModel->addDataModel(session->dataModel());
    m_Controls.treeView->reset();
    m_SelectionProvider->SetItemSelectionModel(m_Controls.treeView->selectionModel());

    connect(session, SIGNAL(progress(QUuid,double)), this, SLOT(OnProgress(QUuid,double)));
    connect(session, SIGNAL(uploadFinished()), this, SLOT(OnProgress(QUuid,double)));
  }
}

void QmitkXnatTreeBrowserView::CleanTreeModel(ctkXnatSession* session)
{
  if (session != 0)
  {
    m_TreeModel->removeDataModel(session->dataModel());
    m_Controls.treeView->reset();
  }
}

void QmitkXnatTreeBrowserView::OnProgress(QUuid /*queryID*/, double progress)
{
  unsigned int currentProgress = progress*100;
  if (currentProgress < 1)
    mitk::ProgressBar::GetInstance()->AddStepsToDo(100);
  else
    mitk::ProgressBar::GetInstance()->Progress();
}

void QmitkXnatTreeBrowserView::InternalFileDownload(const QModelIndex& index, bool loadData)
{
  QVariant variant = m_TreeModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    ctkXnatFile* file = dynamic_cast<ctkXnatFile*>(variant.value<ctkXnatObject*>());
    if (file != NULL)
    {
      QDir downDir(m_DownloadPath);
      QString filePath = m_DownloadPath + file->name();

      // Testing if the file exists already
      if (downDir.exists(file->name()))
      {
        MITK_INFO << "File '" << file->name().toStdString() << "' already exists!";
      }
      else
      {
        if (file->property("collection") == QString("DICOM"))
        {
          ctkXnatObject* parent = file->parent();

          filePath = m_DownloadPath + parent->property("label") + ".zip";
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
          file->download(filePath);

          // Testing if the file exists now
          if (downDir.exists(file->name()))
          {
            MITK_INFO << "Download of " << file->name().toStdString() << " was completed!";
          }
          else
          {
            MITK_INFO << "Download of " << file->name().toStdString() << " failed!";
          }
        }
      }
      if (downDir.exists(file->name()) || file->property("collection") == "DICOM")
      {
        if (loadData)
        {
          if (file->property("collection") == "DICOM")
          {
            // Search for the downloaded file an its file path
            QDirIterator it(m_DownloadPath, QStringList() << file->name(), QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
              it.next();
              filePath = it.filePath();
            }
          }

          if (filePath.isEmpty())
          {
            MITK_INFO << "Decompressing failed!";
            return;
          }
          else if (!QFile(filePath).exists())
          {
            MITK_INFO << "Decompressing failed!";
            return;
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
    }
    else
    {
      MITK_INFO << "Selection was not a file!";
    }
  }
}

void QmitkXnatTreeBrowserView::OnContextMenuDownloadFile()
{
  QModelIndex index = m_Controls.treeView->currentIndex();
  InternalFileDownload(index, false);
}

void QmitkXnatTreeBrowserView::OnContextMenuDownloadAndOpenFile()
{
  QModelIndex index = m_Controls.treeView->currentIndex();
  InternalFileDownload(index, true);
}

void QmitkXnatTreeBrowserView::OnContextMenuCreateResourceFolder()
{
  const QModelIndex index = m_Controls.treeView->selectionModel()->currentIndex();
  ctkXnatObject* parentObject = m_TreeModel->xnatObject(index);

  if (parentObject != nullptr)
  {
    this->InternalAddResourceFolder(parentObject);
  }
}

ctkXnatResource* QmitkXnatTreeBrowserView::InternalAddResourceFolder(ctkXnatObject *parent)
{
  bool ok;
  QString folderName = QInputDialog::getText(m_Controls.treeView, tr("Create XNAT resource folder"),
                                       tr("Folder name:"), QLineEdit::Normal, tr("data"), &ok);
  if (ok)
  {
    if (folderName.isEmpty())
      folderName = "NO LABEL";

    return parent->addResourceFolder(folderName);
  }
  else
  {
    return nullptr;
  }
}

void QmitkXnatTreeBrowserView::InternalFileUpload(ctkXnatFile* file)
{
  try
  {
    file->save();
  }
  catch (ctkXnatException &e)
  {
    QMessageBox msgbox;
    msgbox.setText(e.what());
    msgbox.setIcon(QMessageBox::Critical);
    msgbox.exec();
  }
}

void QmitkXnatTreeBrowserView::OnContextMenuUploadFile()
{
  QString filename = QFileDialog::getOpenFileName(m_Controls.treeView, tr("Open File"), QDir::homePath());
  const QModelIndex index = m_Controls.treeView->selectionModel()->currentIndex();
  ctkXnatResource* resource = dynamic_cast<ctkXnatResource*>(m_TreeModel->xnatObject(index));
  if (resource)
  {
    ctkXnatFile* file = new ctkXnatFile(resource);
    file->setLocalFilePath(filename);
    QFileInfo fileInfo (filename);
    file->setName(fileInfo.fileName());
    file->setFileFormat("some format");
    file->setFileContent("some content");
    file->setFileTags("some, tags");
    file->save();
    m_TreeModel->addChildNode(index, file);
  }
}

#include <ctkXnatDataModel.h>
void QmitkXnatTreeBrowserView::OnUploadResource(const QList<mitk::DataNode*>& droppedNodes, ctkXnatObject* parentObject)
{
  if (parentObject == nullptr)
    return;

  if (dynamic_cast<ctkXnatSession*>(parentObject))
    MITK_INFO<<"SESSION";
  else if (dynamic_cast<ctkXnatDataModel*>(parentObject))
    MITK_INFO<<"DATAMODEL";
  else if (dynamic_cast<ctkXnatProject*>(parentObject))
    MITK_INFO<<"PROJECT";
  else if (dynamic_cast<ctkXnatSubject*>(parentObject))
    MITK_INFO<<"SUBJECT";
  //1. If not dropped on a resource, create a new folder
  //2. Save file locally
  //3. Upload file
  ctkXnatResource* resource = dynamic_cast<ctkXnatResource*>(parentObject);
  if (resource == nullptr)
  {
    resource = this->InternalAddResourceFolder(parentObject);
  }

  if (resource == nullptr)
  {
    MITK_WARN << "Could not upload file! No resource available!";
  }

  mitk::DataNode* node = NULL;
  foreach (node, droppedNodes)
  {
    mitk::BaseData* data = node->GetData();
    if (!data)
      return;


    QString fileName (QString::fromStdString(node->GetName()));

    ctkXnatFile* xnatFile = new ctkXnatFile(resource);

    if (dynamic_cast<mitk::Image*>(data))
    {
      fileName.append(".nrrd");
    }
    else if (dynamic_cast<mitk::Surface*>(data))
    {
      fileName.append(".vtk");
    }
    else if (dynamic_cast<mitk::PointSet*>(data))
    {
      fileName.append(".mps");
    }

    xnatFile->setName(fileName);

    QString xnatFolder = "XNAT_UPLOADS";
    QDir dir(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetContext()->getDataFile("").absoluteFilePath());
    dir.mkdir(xnatFolder);

    fileName = dir.path().append("/" + fileName);
    mitk::IOUtil::Save (data, fileName.toStdString());

    // TODO Check if file exists
    // AbstractFileReader::SetDefaultDataNodeProperties
    // und in die andere SetDefaultDataNodeProperties
    // PropertyName klein: mtime.initial + Kommentar
    mitk::StringProperty::Pointer orignalFilePath = mitk::StringProperty::New();
    node->GetProperty(orignalFilePath, "path");

    xnatFile->setLocalFilePath(fileName);
    //
    xnatFile->setFileFormat("some format");
    xnatFile->setFileContent("some content");
    xnatFile->setFileTags("some, tags");
    //
    xnatFile->save();
    m_TreeModel->addChildNode(m_Controls.treeView->currentIndex(), xnatFile);
    parentObject->fetch();

    // The filename for uploading
//    QFileInfo fileInfo;
    //      if (surface)
    //      {
    //        // Save surface
    //        fileName.append(".stl");
    //        xnatFile->setName(fileName);
    //        dir.setPath(dir.path().append("/" + fileName));

//    QString origFile = QString::fromStdString(orignalFilePath->GetValueAsString());
//    origFile.append("/" + fileName);
    //        origFile.append(".stl");

    //        fileInfo.setFile(origFile);
    //        if (!fileInfo.exists())
    //          mitk::IOUtil::SaveSurface(surface, dir.path().toStdString());
    //      }
//    this->uploadFileToXnat(xnatFile, dir.path());

    // TODO delete file!!!
  }
}

void QmitkXnatTreeBrowserView::OnContextMenuRequested(const QPoint & pos)
{
  m_ContextMenu->clear();
  QModelIndex index = m_Controls.treeView->indexAt(pos);

  ctkXnatObject* xnatObject = m_TreeModel->xnatObject(index);

  bool downloadable = false;
  downloadable |= dynamic_cast<ctkXnatFile*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatScan*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatScanFolder*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatAssessor*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatAssessorFolder*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatResource*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatReconstruction*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatReconstructionFolder*>(xnatObject)!=NULL;

  bool canHaveResourceFolder = false;
  canHaveResourceFolder |= dynamic_cast<ctkXnatProject*>(xnatObject) != NULL;
  canHaveResourceFolder |=  dynamic_cast<ctkXnatSubject*>(xnatObject) != NULL;
  canHaveResourceFolder |=  dynamic_cast<ctkXnatExperiment*>(xnatObject) != NULL;

  bool uploadFilePossible = false;
  uploadFilePossible |= dynamic_cast<ctkXnatResource*>(xnatObject) != NULL;
  uploadFilePossible |=  dynamic_cast<ctkXnatScan*>(xnatObject) != NULL;
  uploadFilePossible |=  dynamic_cast<ctkXnatAssessor*>(xnatObject) != NULL;

  if (downloadable)
  {
    QAction* actDownload = new QAction("Download", m_ContextMenu);
    connect(actDownload, SIGNAL(triggered()), this, SLOT(OnContextMenuDownloadFile()));
    m_ContextMenu->addAction(actDownload);
    ctkXnatFile* file = dynamic_cast<ctkXnatFile*>(xnatObject);
    if (file)
    {
      QAction* actView = new QAction("Download and Open", m_ContextMenu);
      connect(actView, SIGNAL(triggered()), this, SLOT(OnContextMenuDownloadAndOpenFile()));
      m_ContextMenu->addAction(actView);
    }
  }

  if (canHaveResourceFolder)
  {
    QAction* actCreateResource = new QAction("Add resource folder", m_ContextMenu);
    connect(actCreateResource, SIGNAL(triggered()), this, SLOT(OnContextMenuCreateResourceFolder()));
    m_ContextMenu->addAction(actCreateResource);
  }

  if (uploadFilePossible)
  {
    QAction* actUploadFile = new QAction("Upload File", m_ContextMenu);
    connect(actUploadFile, SIGNAL(triggered()), this, SLOT(OnContextMenuUploadFile()));
    m_ContextMenu->addAction(actUploadFile);
  }
  m_ContextMenu->popup(QCursor::pos());
}
