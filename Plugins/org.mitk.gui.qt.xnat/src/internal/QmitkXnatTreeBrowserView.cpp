/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkXnatTreeBrowserView.h"

// Qmitk
#include "org_mitk_gui_qt_xnatinterface_Activator.h"

// Blueberry
#include <berryPlatform.h>

// CTK XNAT Core
#include <ctkXnatAssessor.h>
#include <ctkXnatAssessorFolder.h>
#include <ctkXnatDataModel.h>
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


// MITK XNAT
#include <mitkDataStorage.h>
#include <QmitkIOUtil.h>
#include <QmitkXnatProjectWidget.h>
#include <QmitkXnatSubjectWidget.h>
#include <QmitkXnatExperimentWidget.h>
#include <QmitkXnatCreateObjectDialog.h>
#include <QmitkXnatUploadFromDataStorageDialog.h>

// Qt
#include <QAction>
#include <QClipboard>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QFileInfoList>

#include <QmitkPreferencesDialog.h>

// Poco
#include <Poco/Zip/Decompress.h>

const QString QmitkXnatTreeBrowserView::VIEW_ID = "org.mitk.views.xnat.treebrowser";

static bool isDirWriteable(QDir myDir)
{
  const QFileInfoList tmpInfo = myDir.entryInfoList();
  foreach (QFileInfo f, tmpInfo)
  {
    if(f.fileName() == ".")
      return f.isWritable();
  }
  return true;
}

static bool doesDirExist(QDir myDir)
{
  if (!myDir.exists())
  {
    if(!myDir.mkpath("."))
    {
      MITK_INFO << "Path Creation Failed.";
      return false;
    }
  }
  return true;
}

QmitkXnatTreeBrowserView::QmitkXnatTreeBrowserView() :
  m_DataStorageServiceTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetContext()),
  m_TreeModel(new QmitkXnatTreeModel()),
  m_Tracker(nullptr),
  m_DownloadPath(berry::Platform::GetPreferencesService()->GetSystemPreferences()->Node(VIEW_ID)->Get("Download Path", "")),
  m_SilentMode(false)
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
  m_DataStorageServiceTracker.close();
  delete m_TreeModel;
  delete m_Tracker;
}

void QmitkXnatTreeBrowserView::SetFocus()
{
}

void QmitkXnatTreeBrowserView::FilePathNotAvailableWarning(QString file)
{
  MITK_INFO << "Download of " << file.toStdString() << " failed! Download Path not available!";
  QMessageBox::critical(m_Controls.treeView, "Download failed!", "Download of " + file + " failed!\nDownload Path "
                        + m_DownloadPath + " not available. \n\nChange Download Path in Settings!");
  QmitkPreferencesDialog _PreferencesDialog(QApplication::activeWindow());
  _PreferencesDialog.SetSelectedPage("org.mitk.gui.qt.application.XnatConnectionPreferencePage");
  _PreferencesDialog.exec();
  m_Controls.groupBox->hide();
}

void QmitkXnatTreeBrowserView::ToggleConnection()
{
  ctkXnatSession* session = nullptr;

  try
  {
    session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
          mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
  }
  catch (std::invalid_argument&)
  {
    mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CreateXnatSession();

    session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
          mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());
  }

  if (session != nullptr && session->isOpen())
  {
    mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
    m_Controls.btnXnatConnect->setToolTip("Connect");
    m_Controls.btnXnatConnect->setIcon(QIcon(":/xnat-plugin/xnat-connect.png"));
    CleanUp();
  }
  else if (session != nullptr && !session->isOpen())
  {
    try
    {
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->OpenXnatSession();
      m_Controls.btnXnatConnect->setToolTip("Disconnect");
      m_Controls.btnXnatConnect->setIcon(QIcon(":/xnat-plugin/xnat-disconnect.png"));
      m_Controls.searchField->setEnabled(true);
      m_Controls.searchModeBox->setEnabled(true);
    }
    catch (const ctkXnatAuthenticationException& auth)
    {
      MITK_INFO << auth.message().toStdString();
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
      QmitkPreferencesDialog _PreferencesDialog(QApplication::activeWindow());
      _PreferencesDialog.SetSelectedPage("org.mitk.gui.qt.application.XnatConnectionPreferencePage");
      _PreferencesDialog.exec();
    }
    catch (const ctkException& e)
    {
      MITK_INFO << e.message().toStdString();
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatSessionManager()->CloseXnatSession();
      QmitkPreferencesDialog _PreferencesDialog(QApplication::activeWindow());
      _PreferencesDialog.SetSelectedPage("org.mitk.gui.qt.application.XnatConnectionPreferencePage");
      _PreferencesDialog.exec();
    }
    catch (...)
    {
      QmitkPreferencesDialog _PreferencesDialog(QApplication::activeWindow());
      _PreferencesDialog.SetSelectedPage("org.mitk.gui.qt.application.XnatConnectionPreferencePage");
      _PreferencesDialog.exec();
    }
  }
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

  m_Controls.treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.treeView->setContextMenuPolicy(Qt::CustomContextMenu);

  m_Controls.groupBox->hide();
  m_Controls.wgtExperimentInfo->hide();
  m_Controls.wgtSubjectInfo->hide();
  m_Controls.wgtProjectInfo->hide();

  m_Tracker = new mitk::XnatSessionTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext());

  m_ContextMenu = new QMenu(m_Controls.treeView);

  connect(m_Controls.treeView, SIGNAL(clicked(const QModelIndex&)), SLOT(ItemSelected(const QModelIndex&)));
  connect(m_Controls.treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(OnContextMenuRequested(const QPoint&)));
  connect(m_Tracker, SIGNAL(AboutToBeClosed(ctkXnatSession*)), this, SLOT(CleanTreeModel(ctkXnatSession*)));
  connect(m_Tracker, SIGNAL(Opened(ctkXnatSession*)), this, SLOT(UpdateSession(ctkXnatSession*)));

  m_Tracker->Open();

  connect(m_Controls.treeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnActivatedNode(const QModelIndex&)));
  connect(m_Controls.treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnXnatNodeSelected(const QModelIndex&)));
  connect(m_TreeModel, SIGNAL(ResourceDropped(const QList<mitk::DataNode*>&, ctkXnatObject*, const QModelIndex&)), this, SLOT(OnUploadResource(const QList<mitk::DataNode*>&, ctkXnatObject*, const QModelIndex&)));

  connect(m_Controls.btnXnatConnect, SIGNAL(clicked()), SLOT(ToggleConnection()));
  connect(m_Controls.btnXnatUpload, SIGNAL(clicked()), this, SLOT(OnUploadFromDataStorage()));
  connect(m_Controls.btnXnatDownload, SIGNAL(clicked()), this, SLOT(OnDownloadSelectedXnatFile()));
  connect(m_Controls.btnCreateXnatFolder, SIGNAL(clicked()), this, SLOT(OnCreateResourceFolder()));
  connect(m_Controls.searchField, SIGNAL(textChanged(const QString&)), this, SLOT(Search(const QString&)));
}

void QmitkXnatTreeBrowserView::OnCreateResourceFolder()
{
  QModelIndex index = m_Controls.treeView->selectionModel()->currentIndex();

  if(!index.isValid()) return;

  ctkXnatObject* parent = index.data(Qt::UserRole).value<ctkXnatObject*>();

  this->InternalAddResourceFolder(parent);

  OnContextMenuRefreshItem();
}


void QmitkXnatTreeBrowserView::Search(const QString &toSearch)
{
  if(m_AlreadyInSearch)
    return;

  m_AlreadyInSearch = true;

  m_Controls.treeView->collapseAll();
  m_Controls.treeView->expandToDepth(m_Controls.searchModeBox->currentIndex());

  m_Controls.treeView->clearSelection();

  foreach (const QModelIndex &hidden, m_hiddenItems)
  {
    m_Controls.treeView->setRowHidden(hidden.row(),hidden.parent(),false);
  }
  m_hiddenItems.clear();

  if(toSearch.isEmpty())
  {
    m_Controls.treeView->collapseAll();
    m_Controls.treeView->expandToDepth(0);
    m_AlreadyInSearch = false;
    return;
  }

  QModelIndexList items = m_Controls.treeView->model()->match(
        m_Controls.treeView->model()->index(0,0),
        Qt::DisplayRole,
        QVariant::fromValue(toSearch),
        -1,
        Qt::MatchContains|Qt::MatchRecursive);

  if(!items.isEmpty())
  {
    foreach (const QModelIndex &match, items)
    {
      int depth = 0;
      QModelIndex tparent = match;
      while ( tparent.parent().isValid() )
      {
        tparent = tparent.parent();
        depth++;
      }

      switch (depth)
      {
      case 1: //Project Level
        if(m_Controls.searchModeBox->currentIndex() == ProjectLevel)
        {
          m_hiddenItems.append(match);
          m_Controls.treeView->setRowHidden(match.row(),match.parent(),true);
        }
        break;
      case 2: //Patient level
        if(m_Controls.searchModeBox->currentIndex() == SubjectLevel)
        {
          m_hiddenItems.append(match);
          m_Controls.treeView->setRowHidden(match.row(),match.parent(),true);
        }
        break;
      default:
        break;
      }
    }

  }
  m_AlreadyInSearch = false;
}

void QmitkXnatTreeBrowserView::OnDownloadSelectedXnatFile()
{
  QModelIndex index = m_Controls.treeView->selectionModel()->currentIndex();

  if(!index.isValid()) return;

  ctkXnatObject* selectedXnatObject = index.data(Qt::UserRole).value<ctkXnatObject*>();
  bool enableDownload = dynamic_cast<ctkXnatFile*>(selectedXnatObject) != nullptr;
  enableDownload |= dynamic_cast<ctkXnatScan*>(selectedXnatObject) != nullptr;

  if (enableDownload)
  {
    this->InternalFileDownload(index, true);
  }
}

void QmitkXnatTreeBrowserView::OnUploadFromDataStorage()
{
  QmitkXnatUploadFromDataStorageDialog dialog;
  dialog.SetDataStorage(this->GetDataStorage());
  int result = dialog.exec();
  if (result == QDialog::Accepted)
  {
    QList<mitk::DataNode*> nodes;
    nodes << dialog.GetSelectedNode().GetPointer();
    QModelIndex index = m_Controls.treeView->selectionModel()->currentIndex();

    if (!index.isValid()) return;
    ctkXnatObject* parent = m_TreeModel->xnatObject(index);
    this->OnUploadResource(nodes, parent, index);
  }
}

void QmitkXnatTreeBrowserView::OnXnatNodeSelected(const QModelIndex& index)
{
  if (!index.isValid()) return;

  ctkXnatObject* selectedXnatObject = index.data(Qt::UserRole).value<ctkXnatObject*>();

  // enable download button
  bool enableDownload = dynamic_cast<ctkXnatFile*>(selectedXnatObject) != nullptr;
  enableDownload |= dynamic_cast<ctkXnatScan*>(selectedXnatObject) != nullptr;
  m_Controls.btnXnatDownload->setEnabled(enableDownload);

  // enable folder creation
  bool enableCreateFolder = dynamic_cast<ctkXnatProject*>(selectedXnatObject) != nullptr;
  enableCreateFolder |= dynamic_cast<ctkXnatSubject*>(selectedXnatObject) != nullptr;
  enableCreateFolder |= dynamic_cast<ctkXnatExperiment*>(selectedXnatObject) != nullptr;
  enableCreateFolder |= dynamic_cast<ctkXnatResourceFolder*>(selectedXnatObject) != nullptr;
  m_Controls.btnCreateXnatFolder->setEnabled(enableCreateFolder);

  // enable upload
  bool enableUpload = dynamic_cast<ctkXnatResource*>(selectedXnatObject) != nullptr;
  m_Controls.btnXnatUpload->setEnabled(enableUpload);
}

void QmitkXnatTreeBrowserView::OnActivatedNode(const QModelIndex& index)
{
  if (!index.isValid()) return;

  ctkXnatObject* selectedXnatObject = index.data(Qt::UserRole).value<ctkXnatObject*>();
  bool enableDownload = dynamic_cast<ctkXnatFile*>(selectedXnatObject) != nullptr;
  enableDownload |= dynamic_cast<ctkXnatScan*>(selectedXnatObject) != nullptr;

  if (enableDownload)
  {
    if(!m_SilentMode)
    {
      QMessageBox msgBox;
      QString fname = selectedXnatObject->name() != "" ? selectedXnatObject->name() :
                                                         index.data(Qt::DisplayRole).toString();
      QString msg ("Do you want to download "+ fname +"?");
      msgBox.setWindowTitle("MITK XNAT download");
      msgBox.setText(msg);
      msgBox.setIcon(QMessageBox::Question);
      msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
      int result = msgBox.exec();
      if (result == QMessageBox::Ok)
      {
        InternalFileDownload(index, true);
      }
    }
    else
    {
      InternalFileDownload(index, true);
    }
  }
}

void QmitkXnatTreeBrowserView::UpdateSession(ctkXnatSession* session)
{
  if (session != nullptr && session->isOpen())
  {
    // Fill model and show in the GUI
    m_TreeModel->addDataModel(session->dataModel());
    m_Controls.treeView->reset();

    connect(session, SIGNAL(progress(QUuid,double)), this, SLOT(OnProgress(QUuid,double)));
    connect(session, SIGNAL(timedOut()), this, SLOT(SessionTimedOutMsg()));
    connect(session, SIGNAL(aboutToTimeOut()), this, SLOT(SessionTimesOutSoonMsg()));
  }
}

void QmitkXnatTreeBrowserView::CleanTreeModel(ctkXnatSession* session)
{
  if (session != nullptr)
  {
    m_TreeModel->removeDataModel(session->dataModel());
    m_Controls.treeView->reset();
  }
}

void QmitkXnatTreeBrowserView::OnProgress(QUuid /*queryID*/, double progress)
{
  if (progress > 0)
  {
    unsigned int currentProgress = progress*100;
    if (m_Controls.groupBox->isHidden())
    {
      m_Controls.groupBox->show();
      m_Controls.progressBar->setValue(0);
    }
    m_Controls.progressBar->setValue(currentProgress);
  }
}

void QmitkXnatTreeBrowserView::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  QString downloadPath = prefs->Get("Download Path", "");
  QDir downloadDir (downloadPath);
  if (downloadPath.length() != 0 && downloadDir.exists())
    m_DownloadPath = downloadPath;

  m_SilentMode = prefs->GetBool("Silent Mode", false);
}

void QmitkXnatTreeBrowserView::InternalFileDownload(const QModelIndex& index, bool loadData)
{
  if (!index.isValid())
    return;

  QDir rootDownloadDir(m_DownloadPath);
  if(isDirWriteable(rootDownloadDir) == false)
  {
    MITK_INFO << "Download directory access permission unsufficient! " << m_DownloadPath;
    QMessageBox::critical(nullptr,"Download directory access permission unsufficient!",
                          "You have no permission to write to selected download directory " + m_DownloadPath +"!\n\nChange Download Path in Settings!");
    QmitkPreferencesDialog _PreferencesDialog(QApplication::activeWindow());
    _PreferencesDialog.SetSelectedPage("org.mitk.gui.qt.application.XnatConnectionPreferencePage");
    _PreferencesDialog.exec();
    return;
  }

  ctkXnatObject* xnatObject = m_TreeModel->xnatObject(index);
  if (xnatObject != nullptr)
  {
    m_Controls.progressBar->setMinimum(0);
    m_Controls.progressBar->setMaximum(100);
    // The path to the downloaded file
    QString filePath;
    QDir downloadPath (m_DownloadPath);
    QString serverURL = berry::Platform::GetPreferencesService()->GetSystemPreferences()->Node(VIEW_ID)->Get("Server Address", "");
    bool isDICOM (false);
    bool filePathExists (true);

    // If a scan was selected, downloading the contained DICOM folder as ZIP
    ctkXnatScan* scan = dynamic_cast<ctkXnatScan*>(xnatObject);

    if (scan != nullptr)
    {
      isDICOM = true;

      if (!scan->isFetched())
        scan->fetch();

      QList<ctkXnatObject*> children = scan->children();

      foreach (ctkXnatObject* obj, children)
      {
        if (obj->name() == "DICOM")
        {
          QString uriId = obj->resourceUri();
          uriId.replace("/data/archive/projects/", "");
          QString folderName = m_DownloadPath + uriId + "/";
          downloadPath = folderName;
          filePathExists = doesDirExist(downloadPath);

          if(filePathExists)
          {
            try
            {
              this->InternalDICOMDownload(obj, downloadPath);
            }
            catch(const ctkRuntimeException& exc)
            {
              QmitkHttpStatusCodeHandler::HandleErrorMessage(exc.what());
              return;
            }
            serverURL = obj->resourceUri();
          }
          else
          {
            FilePathNotAvailableWarning("DICOM folder");
            return;
          }
        }
      }
    }
    else
    {
      ctkXnatFile* file = dynamic_cast<ctkXnatFile*>(xnatObject);

      if (file == nullptr)
      {
        MITK_ERROR << "Selected XNAT object not downloadable!";
        return;
      }

      QString uriId = file->parent()->resourceUri();
      uriId.replace("/data/archive/projects/", "");

      QString folderName = m_DownloadPath + uriId + "/";
      downloadPath = folderName;

      filePathExists = doesDirExist(downloadPath);
      filePath = folderName + file->name();

      // Checking if the file exists already
      if (downloadPath.exists(file->name()))
      {
        MITK_INFO << "File '" << file->name().toStdString() << "' already exists!";
        serverURL = file->parent()->resourceUri();
      }
      else
      {
        if (file->property("collection") == QString("DICOM"))
        {
          isDICOM = true;
          ctkXnatObject* parent = file->parent();

          QString uriId = parent->resourceUri();
          uriId.replace("/data/archive/projects/", "");
          QString folderName = m_DownloadPath + uriId + "/";
          downloadPath = folderName;
          filePathExists = doesDirExist(downloadPath);

          if(filePathExists)
          {
            try
            {
              this->InternalDICOMDownload(parent, downloadPath);
            }
            catch(const ctkRuntimeException& exc)
            {
              QmitkHttpStatusCodeHandler::HandleErrorMessage(exc.what());
              return;
            }
          }
          else
          {
            FilePathNotAvailableWarning(parent->name());
            return;
          }

          serverURL = parent->resourceUri();
        }
        //Normal file download, no DICOM download
        else
        {
          if(filePathExists)
          {
            this->SetStatusInformation("Downloading file " + file->name());
            file->download(filePath);
          }
          else
          {
            MITK_INFO << "File Download Failed.";
          }
          serverURL = file->parent()->resourceUri();

          // Checking if the file exists now
          if (downloadPath.exists(file->name()))
          {
            MITK_INFO << "Download of " << file->name().toStdString() << " completed!";
            if(!m_SilentMode)
            {
              QMessageBox msgBox;
              msgBox.setText("Download of " + file->name() + " completed!");
              msgBox.setIcon(QMessageBox::Information);
              msgBox.exec();
            }
            m_Controls.groupBox->hide();
          }
          else
          {
            if(filePathExists)
            {
              MITK_INFO << "Download of " << file->name().toStdString() << " failed!";
              QMessageBox::critical(m_Controls.treeView, "Download failed!", "Download of " + file->name() + " failed!");
              m_Controls.groupBox->hide();
              return;
            }
            else
            {
              FilePathNotAvailableWarning(file->name());
              return;
            }
          }
        }
      }
    }
    if (loadData)
    {
      QFileInfoList fileList;
      if (isDICOM)
      {
        fileList = downloadPath.entryInfoList(QDir::Files);
      }
      else
      {
        QFileInfo fileInfo(filePath);
        fileList << fileInfo;
      }

      mitk::StringProperty::Pointer xnatURL = mitk::StringProperty::New(serverURL.toStdString());
      try
      {
        this->InternalOpenFiles(fileList, xnatURL);
      }
      catch(const ctkRuntimeException& exc)
      {
        QmitkHttpStatusCodeHandler::HandleErrorMessage(exc.what());
        return;
      }
    }
  }
}

void QmitkXnatTreeBrowserView::InternalDICOMDownload(ctkXnatObject *obj, QDir &DICOMDirPath)
{

  QString filePath = m_DownloadPath + obj->property("label") + ".zip";

  this->SetStatusInformation("Downloading DICOM series " + obj->parent()->name());
  // In case of DICOM zip download we do not know the total file size
  // Because of that the dowload progres cannot be calculated
  // Because of that we use the busy indicator of the progress bar by setting min and max to 0
  m_Controls.progressBar->setMinimum(0);
  m_Controls.progressBar->setMaximum(0);
  m_Controls.progressBar->show();
  obj->download(filePath);

  std::ifstream in(filePath.toStdString().c_str(), std::ios::binary);
  poco_assert(in);

  // decompress to XNAT_DOWNLOAD dir
  Poco::Zip::Decompress dec(in, Poco::Path(DICOMDirPath.path().toStdString()), true);
  dec.decompressAllFiles();

  in.close();
  QFile::remove(filePath);

  // Checking if the file exists now
  if (DICOMDirPath.exists())
  {
    if(!m_SilentMode)
    {
      MITK_INFO << "Download of DICOM series completed!";
      QMessageBox msgBox;
      msgBox.setText("Download of DICOM series completed!");
      msgBox.setIcon(QMessageBox::Information);
      msgBox.exec();
      m_Controls.groupBox->hide();
    }
  }
  else
  {
    MITK_INFO << "Download of DICOM series failed!";
    QMessageBox msgBox;
    msgBox.setText("Download of DICOM series failed!");
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
    m_Controls.groupBox->hide();
  }
}

void QmitkXnatTreeBrowserView::InternalOpenFiles(const QFileInfoList & fileList, mitk::StringProperty::Pointer xnatURL)
{

  if (fileList.isEmpty())
  {
    MITK_WARN << "No files available for loading!";
    return;
  }

  mitk::IDataStorageService* dsService = m_DataStorageServiceTracker.getService();
  mitk::DataStorage::Pointer dataStorage = dsService->GetDataStorage()->GetDataStorage();
  QStringList list;
  list << fileList.at(0).absoluteFilePath();
  try
  {
    mitk::DataStorage::SetOfObjects::Pointer nodes = QmitkIOUtil::Load(list, *dataStorage);
    if (nodes->size() == 1)
    {
      mitk::DataNode* node = nodes->at(0);
      node->SetProperty("xnat.url", xnatURL);
    }
  }
  catch (const mitk::Exception& e)
  {
    MITK_INFO << e;
    return;
  }
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(
        dsService->GetDataStorage()->GetDataStorage());
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

    // if called on the resource-folder level we need to provide he corresponding
    // parent instead of the folder
    ctkXnatResourceFolder* resourceFolder = dynamic_cast<ctkXnatResourceFolder*>(parent);
    if (resourceFolder != nullptr)
    {
      parent = parent->parent();
    }

    try
    {
      return parent->addResourceFolder(folderName);
    }
    catch(const ctkRuntimeException& exc)
    {
      QmitkHttpStatusCodeHandler::HandleErrorMessage(exc.what());
      return nullptr;
    }
  }
  else
  {
    return nullptr;
  }
}

void QmitkXnatTreeBrowserView::InternalFileUpload(ctkXnatFile* file)
{
  m_Controls.groupBox->setTitle("Uploading file...");
  m_Controls.groupBox->show();

  try
  {
    file->save();
    MITK_INFO << "Upload of " << file->name().toStdString() << " completed!";
    if(!m_SilentMode)
    {
      QMessageBox msgBox;
      msgBox.setText("Upload of " + file->name() + " completed!");
      msgBox.setIcon(QMessageBox::Information);
      msgBox.show();
      msgBox.exec();
    }
  }
  catch (ctkXnatException &e)
  {
    QMessageBox msgbox;
    msgbox.setText(e.what());
    msgbox.setIcon(QMessageBox::Critical);
    msgbox.exec();
    m_Controls.progressBar->setValue(0);
  }
  m_Controls.groupBox->hide();
}

void QmitkXnatTreeBrowserView::OnContextMenuUploadFile()
{
  QString filename = QFileDialog::getOpenFileName(m_Controls.treeView, tr("Open File"), QDir::homePath());
  const QModelIndex index = m_Controls.treeView->selectionModel()->currentIndex();
  ctkXnatResource* resource = dynamic_cast<ctkXnatResource*>(m_TreeModel->xnatObject(index));
  if (resource != nullptr && filename.length() != 0)
  {
    ctkXnatFile* file = new ctkXnatFile(resource);
    file->setLocalFilePath(filename);
    QFileInfo fileInfo (filename);
    file->setName(fileInfo.fileName());
    try
    {
      this->InternalFileUpload(file);
    }
    catch(const ctkRuntimeException& exc)
    {
      QmitkHttpStatusCodeHandler::HandleErrorMessage(exc.what());
      return;
    }
    m_TreeModel->addChildNode(index, file);
  }
}

void QmitkXnatTreeBrowserView::OnContextMenuCopyXNATUrlToClipboard()
{
  const QModelIndex index = m_Controls.treeView->selectionModel()->currentIndex();
  ctkXnatObject* currentXnatObject = m_TreeModel->xnatObject(index);
  if (currentXnatObject != nullptr)
  {
    QString serverURL = berry::Platform::GetPreferencesService()->GetSystemPreferences()->Node(VIEW_ID)->Get("Server Address", "");
    serverURL.append(currentXnatObject->resourceUri());
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(serverURL);
  }
}

void QmitkXnatTreeBrowserView::OnContextMenuRefreshItem()
{
  const QModelIndex index = m_Controls.treeView->selectionModel()->currentIndex();
  if (index.isValid())
  {
    this->m_TreeModel->refresh(index);
  }
}

void QmitkXnatTreeBrowserView::OnUploadResource(const QList<mitk::DataNode*>& droppedNodes, ctkXnatObject* parentObject, const QModelIndex& parentIndex)
{
  if (parentObject == nullptr)
    return;

  //1. If not dropped on a resource, create a new folder
  //temporarily remove the annoying message box that upload was successful..
  ctkXnatResource* resource = dynamic_cast<ctkXnatResource*>(parentObject);
  // store original resource folder object for later use
  ctkXnatResourceFolder* originalResourceFolder = dynamic_cast<ctkXnatResourceFolder*>(parentObject);
  if (resource == nullptr)
  {
    resource = this->InternalAddResourceFolder(parentObject);
  }

  if (resource == nullptr)
  {
    MITK_WARN << "Resource folder could not be created!";
    return;
  }

  //2. Save files locally
  //3. Upload file
  mitk::DataNode* node = nullptr;
  foreach (node, droppedNodes)
  {
    mitk::BaseData* data = node->GetData();
    if (!data)
      return;

    //We have to replace special characters due to XNAT inability to get along with them (" " is replaced by "%20", what leads to nasty behaviour!)
    QString fileName(QString::fromStdString(ReplaceSpecialChars(node->GetName())));

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
    else
    {
      MITK_WARN << "Could not upload file! File-type not supported";
      QMessageBox msgbox;
      msgbox.setText("Could not upload file! File-type not supported");
      msgbox.setIcon(QMessageBox::Critical);
      msgbox.exec();
      return;
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

    try
    {
      this->InternalFileUpload(xnatFile);
    }
    catch(const ctkRuntimeException& exc)
    {
      QmitkHttpStatusCodeHandler::HandleErrorMessage(exc.what());
      return;
    }
    QFile::remove(fileName);

    if (originalResourceFolder == nullptr)
    {
      m_TreeModel->refresh(parentIndex);
    }
    else
    {
      m_TreeModel->refresh(parentIndex.parent());
    }

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
    //          mitk::IOUtil::Save(surface, dir.path().toStdString());
    //      }
    //    this->uploadFileToXnat(xnatFile, dir.path());
  }
}

void QmitkXnatTreeBrowserView::OnContextMenuRequested(const QPoint & pos)
{
  if(m_TreeModel==nullptr)
  {
    return;
  }

  QModelIndex index = m_Controls.treeView->indexAt(pos);

  if(index.isValid() == false)
  {
    return;
  }

  m_ContextMenu->clear();

  QAction* actRefreshItem = new QAction("Refresh", m_ContextMenu);
  m_ContextMenu->addAction(actRefreshItem);
  connect(actRefreshItem, SIGNAL(triggered()), this, SLOT(OnContextMenuRefreshItem()));

  QAction* actGetXNATURL = new QAction("Copy XNAT URL to clipboard", m_ContextMenu);
  m_ContextMenu->addAction(actGetXNATURL);
  connect(actGetXNATURL, SIGNAL(triggered()), this, SLOT(OnContextMenuCopyXNATUrlToClipboard()));
  m_ContextMenu->addSeparator();

  ctkXnatObject* xnatObject = m_TreeModel->xnatObject(index);

  bool downloadable = false;
  downloadable |= dynamic_cast<ctkXnatFile*>(xnatObject)!=nullptr;
  downloadable |= dynamic_cast<ctkXnatScan*>(xnatObject)!=nullptr;
  downloadable |= dynamic_cast<ctkXnatScanFolder*>(xnatObject)!=nullptr;
  downloadable |= dynamic_cast<ctkXnatAssessor*>(xnatObject)!=nullptr;
  downloadable |= dynamic_cast<ctkXnatAssessorFolder*>(xnatObject)!=nullptr;
  downloadable |= dynamic_cast<ctkXnatResource*>(xnatObject)!=nullptr;
  downloadable |= dynamic_cast<ctkXnatReconstruction*>(xnatObject)!=nullptr;
  downloadable |= dynamic_cast<ctkXnatReconstructionFolder*>(xnatObject)!=nullptr;

  bool canHaveResourceFolder = false;
  canHaveResourceFolder |= dynamic_cast<ctkXnatProject*>(xnatObject) != nullptr;
  canHaveResourceFolder |=  dynamic_cast<ctkXnatSubject*>(xnatObject) != nullptr;
  canHaveResourceFolder |=  dynamic_cast<ctkXnatExperiment*>(xnatObject) != nullptr;
  canHaveResourceFolder |= dynamic_cast<ctkXnatResourceFolder*>(xnatObject) != nullptr;

  bool uploadFilePossible = false;
  uploadFilePossible |= dynamic_cast<ctkXnatResource*>(xnatObject) != nullptr;
  uploadFilePossible |=  dynamic_cast<ctkXnatScan*>(xnatObject) != nullptr;
  uploadFilePossible |=  dynamic_cast<ctkXnatAssessor*>(xnatObject) != nullptr;

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

  ctkXnatProject* project = dynamic_cast<ctkXnatProject*>(xnatObject);
  if (project != nullptr)
  {
    QAction* actCreateSubject = new QAction("Create new subject", m_ContextMenu);
    m_ContextMenu->addAction(actCreateSubject);
    connect(actCreateSubject, SIGNAL(triggered()), this, SLOT(OnContextMenuCreateNewSubject()));
  }
  ctkXnatSubject* subject = dynamic_cast<ctkXnatSubject*>(xnatObject);
  if (subject != nullptr)
  {
    QAction* actCreateExperiment = new QAction("Create new experiment", m_ContextMenu);
    m_ContextMenu->addAction(actCreateExperiment);
    connect(actCreateExperiment, SIGNAL(triggered()), this, SLOT(OnContextMenuCreateNewExperiment()));
  }


  m_ContextMenu->popup(QCursor::pos());

}

void QmitkXnatTreeBrowserView::CleanUp()
{
  m_Controls.wgtExperimentInfo->hide();
  m_Controls.wgtSubjectInfo->hide();
  m_Controls.wgtProjectInfo->hide();
  m_Controls.btnCreateXnatFolder->setEnabled(false);
  m_Controls.btnXnatDownload->setEnabled(false);
  m_Controls.btnXnatUpload->setEnabled(false);
  m_Controls.searchField->setEnabled(false);
  m_Controls.searchField->setText("");
  m_Controls.searchModeBox->setEnabled(false);
}

std::string QmitkXnatTreeBrowserView::ReplaceSpecialChars(const std::string& input) const
{
  QString convertedString = QString(QUrl::toPercentEncoding(QString::fromStdString(input)));
  return  convertedString.toStdString();
}

void QmitkXnatTreeBrowserView::ItemSelected(const QModelIndex& index)
{
  //TODO: CTK seems to ignore spaces while creating the http request. This will lead to corrupted http request that will fail.
  QVariant variant = m_TreeModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    ctkXnatSession *session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
          mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());

    ctkXnatObject* object = variant.value<ctkXnatObject*>();
    ctkXnatProject* project = dynamic_cast<ctkXnatProject*>(object);
    ctkXnatSubject* subject = dynamic_cast<ctkXnatSubject*>(object);
    ctkXnatExperiment* experiment = dynamic_cast<ctkXnatExperiment*>(object);

    if (project != nullptr)
    {
      m_Controls.wgtExperimentInfo->hide();
      m_Controls.wgtSubjectInfo->hide();

      m_Controls.wgtProjectInfo->SetProject(project);
      m_Controls.wgtProjectInfo->show();
    }
    else if (subject != nullptr)
    {
      QMap<QString, QString> paramMap;
      paramMap.insert("columns", "dob,gender,handedness,weight,height");
      QUuid requestID = session->httpGet(QString("%1/subjects").arg(subject->parent()->resourceUri()), paramMap);
      QList<QVariantMap> results = session->httpSync(requestID);

      foreach(const QVariantMap& propertyMap, results)
      {
        QMapIterator<QString, QVariant> it(propertyMap);
        bool isConcretSubject = false;

        if (it.hasNext())
        {
          it.next();

          QVariant var = it.value();

          // After CTK Change (subjectID = name) to (subjectID = ID)
          // CHANGE TO: if (var == subject->property("ID"))
          if (var == subject->property("URI").right(11))
          {
            isConcretSubject = true;
          }
          else
          {
            isConcretSubject = false;
          }
          it.toFront();
        }

        while (it.hasNext() && isConcretSubject)
        {
          it.next();

          QString  str = it.key().toLatin1().data();
          QVariant var = it.value();

          subject->setProperty(str, var);
        }
      }
      m_Controls.wgtExperimentInfo->hide();
      m_Controls.wgtProjectInfo->hide();

      m_Controls.wgtSubjectInfo->SetSubject(subject);
      m_Controls.wgtSubjectInfo->show();
    }
    else if (experiment != nullptr)
    {
      QMap<QString, QString> paramMap;
      paramMap.insert("columns", "date,time,scanner,modality");
      QUuid requestID = session->httpGet(QString("%1/experiments").arg(experiment->parent()->resourceUri()), paramMap);
      QList<QVariantMap> results = session->httpSync(requestID);

      foreach(const QVariantMap& propertyMap, results)
      {
        QMapIterator<QString, QVariant> it(propertyMap);
        bool isConcretExperiment = false;

        if (it.hasNext())
        {
          it.next();

          QVariant var = it.value();

          if (var == experiment->property("URI"))
          {
            isConcretExperiment = true;
          }
          else
          {
            isConcretExperiment = false;
          }
          it.toFront();
        }

        while (it.hasNext() && isConcretExperiment)
        {
          it.next();

          QString  str = it.key().toLatin1().data();
          QVariant var = it.value();

          experiment->setProperty(str, var);
        }
      }

      m_Controls.wgtSubjectInfo->hide();
      m_Controls.wgtProjectInfo->hide();

      m_Controls.wgtExperimentInfo->SetExperiment(experiment);
      m_Controls.wgtExperimentInfo->show();
    }
  }
}

void QmitkXnatTreeBrowserView::OnContextMenuCreateNewSubject()
{
  QModelIndex index = m_Controls.treeView->currentIndex();
  QVariant variant = m_TreeModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    QmitkXnatCreateObjectDialog* dialog = new QmitkXnatCreateObjectDialog(QmitkXnatCreateObjectDialog::SpecificType::SUBJECT);
    if (dialog->exec() == QDialog::Accepted)
    {
      ctkXnatProject* project = dynamic_cast<ctkXnatProject*>(variant.value<ctkXnatObject*>());
      ctkXnatSubject* subject = dynamic_cast<ctkXnatSubject*>(dialog->GetXnatObject());
      subject->setParent(project);
      try
      {
        subject->save();
      }
      catch(const ctkRuntimeException& exc)
      {
        //TODO: Implement isValid-flag to check if ctkRuntimeExceptio is valid http-exception.
        QmitkHttpStatusCodeHandler::HandleErrorMessage(exc.what());
        project->remove(subject);
        delete subject;
        return;
      }

      // Get xnat session from micro service
      ctkXnatSession* session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
            mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());

      // Update View
      m_TreeModel->removeDataModel(session->dataModel());

      UpdateSession(session);
    }
  }
}

void QmitkXnatTreeBrowserView::OnContextMenuCreateNewExperiment()
{
  QModelIndex index = m_Controls.treeView->currentIndex();
  QVariant variant = m_TreeModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    QmitkXnatCreateObjectDialog* dialog = new QmitkXnatCreateObjectDialog(QmitkXnatCreateObjectDialog::SpecificType::EXPERIMENT);
    if (dialog->exec() == QDialog::Accepted)
    {
      ctkXnatSubject* subject = dynamic_cast<ctkXnatSubject*>(variant.value<ctkXnatObject*>());
      ctkXnatExperiment* experiment = dynamic_cast<ctkXnatExperiment*>(dialog->GetXnatObject());
      experiment->setParent(subject);
      experiment->setProperty("xsiType", experiment->imageModality());

      try
      {
        experiment->save();
      }
      catch(const ctkRuntimeException& exc)
      {
        QmitkHttpStatusCodeHandler::HandleErrorMessage(exc.what());
        subject->remove(experiment);
        delete experiment;
        return;
      }

      // Get xnat session from micro service
      ctkXnatSession* session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
            mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());

      // Update View
      m_TreeModel->removeDataModel(session->dataModel());
      UpdateSession(session);
    }
  }
}

void QmitkXnatTreeBrowserView::SetStatusInformation(const QString& text)
{
  m_Controls.groupBox->setTitle(text);
  m_Controls.progressBar->setValue(0);
  m_Controls.groupBox->show();
}

void QmitkXnatTreeBrowserView::SessionTimedOutMsg()
{
  ctkXnatSession* session = qobject_cast<ctkXnatSession*>(QObject::sender());

  if (session == nullptr)
    return;

  ctkXnatDataModel* dataModel = session->dataModel();
  m_TreeModel->removeDataModel(dataModel);
  m_Controls.treeView->reset();
  session->close();
  QMessageBox::warning(m_Controls.treeView, "Session Timeout", "The session timed out.");

  m_Controls.btnXnatConnect->setToolTip("Connect");
  m_Controls.btnXnatConnect->setIcon(QIcon(":/xnat-plugin/xnat-connect.png"));

  CleanUp();
}

void QmitkXnatTreeBrowserView::SessionTimesOutSoonMsg()
{
  ctkXnatSession* session = qobject_cast<ctkXnatSession*>(QObject::sender());

  if (session == nullptr)
    return;

  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Warning);
  msgBox.setWindowTitle("Session Timeout Soon");
  msgBox.setText("The session will time out in 1 minute.\nDo you want to renew the session?");
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.show();
  QTimer* timer = new QTimer(this);
  timer->start(60000);
  this->connect(timer, SIGNAL(timeout()), &msgBox, SLOT(reject()));
  if (msgBox.exec() == QMessageBox::Yes){
    session->renew();
  }
  timer->stop();
}
