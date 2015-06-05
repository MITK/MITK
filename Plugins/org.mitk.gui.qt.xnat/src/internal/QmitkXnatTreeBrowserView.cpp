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
#include <ctkXnatFile.h>
#include <ctkXnatProject.h>
#include <ctkXnatSubject.h>
#include <ctkXnatExperiment.h>

// MITK XNAT
#include <QmitkXnatProjectWidget.h>
#include <QmitkXnatSubjectWidget.h>
#include <QmitkXnatExperimentWidget.h>
#include <QmitkXnatCreateObjectDialog.h>

// Qt
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QDirIterator>
#include <QDialog>
#include <QlayoutItem>
#include <QDialogButtonBox>

// MITK
#include <mitkDataStorage.h>
#include <QmitkIOUtil.h>

// Poco
#include <Poco/Zip/Decompress.h>

const std::string QmitkXnatTreeBrowserView::VIEW_ID = "org.mitk.views.xnat.treebrowser";

QmitkXnatTreeBrowserView::QmitkXnatTreeBrowserView() :
m_TreeModel(new ctkXnatTreeModel()),
m_Tracker(0),
m_DownloadPath(berry::Platform::GetPreferencesService()->GetSystemPreferences()->Node("/XnatConnection")->Get("Download Path", "")),
m_DataStorageServiceTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetContext())
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

void QmitkXnatTreeBrowserView::CreateQtPartControl(QWidget *parent)
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  m_Controls.treeView->setModel(m_TreeModel);
  m_Controls.treeView->header()->hide();
  m_Controls.labelError->setText("Please use the 'Connect' button in the Preferences.");
  m_Controls.labelError->setStyleSheet("QLabel { color: red; }");

  m_SelectionProvider = new berry::QtSelectionProvider();
  this->SetSelectionProvider();
  m_Controls.treeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.treeView->setContextMenuPolicy(Qt::CustomContextMenu);

  m_Tracker = new mitk::XnatSessionTracker(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext());
  m_NodeMenu = new QMenu(m_Controls.treeView);

  connect(m_Controls.treeView, SIGNAL(clicked(const QModelIndex&)), SLOT(itemSelected(const QModelIndex&)));
  connect(m_Controls.treeView, SIGNAL(customContextMenuRequested(const QPoint&)),
    this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)));
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

void QmitkXnatTreeBrowserView::NodeTableViewContextMenuRequested(const QPoint & pos)
{
  m_NodeMenu->clear();
  QModelIndex index = m_Controls.treeView->indexAt(pos);
  QVariant variant = m_TreeModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    ctkXnatFile* file = dynamic_cast<ctkXnatFile*>(variant.value<ctkXnatObject*>());
    if (file != NULL)
    {
      QAction* actShow = new QAction("Download and Open", m_NodeMenu);
      QAction* actDownload = new QAction("Download", m_NodeMenu);
      m_NodeMenu->addAction(actShow);
      m_NodeMenu->addAction(actDownload);
      connect(actShow, SIGNAL(triggered()), this, SLOT(OnContextMenuDownloadAndOpenFile()));
      connect(actDownload, SIGNAL(triggered()), this, SLOT(OnContextMenuDownloadFile()));
      m_NodeMenu->popup(QCursor::pos());
    }
    ctkXnatProject* project = dynamic_cast<ctkXnatProject*>(variant.value<ctkXnatObject*>());
    if (project != NULL)
    {
      QAction* actCreateSubject = new QAction("Create new subject", m_NodeMenu);
      m_NodeMenu->addAction(actCreateSubject);
      connect(actCreateSubject, SIGNAL(triggered()), this, SLOT(OnContextMenuCreateNewSubject()));
      m_NodeMenu->popup(QCursor::pos());
    }
    ctkXnatSubject* subject = dynamic_cast<ctkXnatSubject*>(variant.value<ctkXnatObject*>());
    if (subject != NULL)
    {
      QAction* actCreateExperiment = new QAction("Create new experiment", m_NodeMenu);
      m_NodeMenu->addAction(actCreateExperiment);
      connect(actCreateExperiment, SIGNAL(triggered()), this, SLOT(OnContextMenuCreateNewExperiment()));
      m_NodeMenu->popup(QCursor::pos());
    }
  }
}

void QmitkXnatTreeBrowserView::itemSelected(const QModelIndex& index)
{
  QLayout* layout = m_Controls.infoVerticalLayout;
  QLayoutItem *child;
  while ((child = layout->takeAt(0)) != 0) {
    delete child->widget();
  }

  QVariant variant = m_TreeModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    ctkXnatSession *session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
      mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());

    ctkXnatObject* object = variant.value<ctkXnatObject*>();
    ctkXnatProject* project = dynamic_cast<ctkXnatProject*>(object);
    ctkXnatSubject* subject = dynamic_cast<ctkXnatSubject*>(object);
    ctkXnatExperiment* experiment = dynamic_cast<ctkXnatExperiment*>(object);

    if (project != NULL)
    {
      QmitkXnatProjectWidget* widget = new QmitkXnatProjectWidget(QmitkXnatProjectWidget::Mode::INFO);
      widget->SetProject(project);
      layout->addWidget(widget);
    }
    else if (subject != NULL)
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

          QString  str = it.key().toLatin1().data();
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

      QmitkXnatSubjectWidget* widget = new QmitkXnatSubjectWidget(QmitkXnatSubjectWidget::Mode::INFO);
      widget->SetSubject(subject);
      layout->addWidget(widget);
    }
    else if (experiment != NULL)
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

          QString  str = it.key().toLatin1().data();
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

      QmitkXnatExperimentWidget* widget = new QmitkXnatExperimentWidget(QmitkXnatExperimentWidget::Mode::INFO);
      widget->SetExperiment(experiment);
      layout->addWidget(widget);
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
      subject->setId(subject->label());
      subject->save();

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
      experiment->setId(experiment->label());
      experiment->setProperty("project", QString(subject->parent()->property("ID")));
      experiment->setProperty("xsiType", QString("xnat:crSessionData"));
      experiment->save();

      // Get xnat session from micro service
      ctkXnatSession* session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetService(
        mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>());

      // Update View
      m_TreeModel->removeDataModel(session->dataModel());
      UpdateSession(session);
    }
  }
}
