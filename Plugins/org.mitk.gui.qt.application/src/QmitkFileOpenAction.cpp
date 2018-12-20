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

#include "QmitkFileOpenAction.h"

#include "internal/org_mitk_gui_qt_application_Activator.h"

#include <mitkIDataStorageService.h>
#include <mitkNodePredicateProperty.h>
#include <mitkWorkbenchUtil.h>

#include <QmitkIOUtil.h>

#include <berryIPreferences.h>

#include <QFileDialog>

namespace
{
  mitk::DataStorage::Pointer GetDataStorage()
  {
    auto context = mitk::org_mitk_gui_qt_application_Activator::GetContext();

    if (nullptr == context)
      return nullptr;

    auto dataStorageServiceReference = context->getServiceReference<mitk::IDataStorageService>();

    if (!dataStorageServiceReference)
      return nullptr;

    auto dataStorageService = context->getService<mitk::IDataStorageService>(dataStorageServiceReference);

    if (nullptr == dataStorageService)
      return nullptr;

    auto dataStorageReference = dataStorageService->GetDataStorage();

    if (dataStorageReference.IsNull())
      return nullptr;

    return dataStorageReference->GetDataStorage();
  }

  mitk::DataNode::Pointer GetFirstSelectedNode()
  {
    auto dataStorage = GetDataStorage();

    if (dataStorage.IsNull())
      return nullptr;

    auto selectedNodes = dataStorage->GetSubset(mitk::NodePredicateProperty::New("selected", mitk::BoolProperty::New(true)));

    if (selectedNodes->empty())
      return nullptr;

    return selectedNodes->front();
  }

  QString GetPathOfFirstSelectedNode()
  {
    auto firstSelectedNode = GetFirstSelectedNode();

    if (firstSelectedNode.IsNull())
      return "";

    auto data = firstSelectedNode->GetData();

    if (nullptr == data)
      return "";

    auto pathProperty = data->GetConstProperty("path");

    if (pathProperty.IsNull())
      return "";

    return QFileInfo(QString::fromStdString(pathProperty->GetValueAsString())).canonicalPath();
  }
}

class QmitkFileOpenActionPrivate
{
public:

  void init ( berry::IWorkbenchWindow* window, QmitkFileOpenAction* action )
  {
    m_Window = window;
    action->setText("&Open File...");
    action->setToolTip("Open data files (images, surfaces,...)");

    QObject::connect(action, SIGNAL(triggered(bool)), action, SLOT(Run()));
  }

  berry::IPreferences::Pointer GetPreferences() const
  {
    berry::IPreferencesService* prefService = mitk::PluginActivator::GetInstance()->GetPreferencesService();
    if (prefService)
    {
      return prefService->GetSystemPreferences()->Node("/General");
    }
    return berry::IPreferences::Pointer(nullptr);
  }

  QString getLastFileOpenPath() const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if(prefs.IsNotNull())
    {
      return prefs->Get("LastFileOpenPath", "");
    }
    return QString();
  }

  void setLastFileOpenPath(const QString& path) const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if(prefs.IsNotNull())
    {
      prefs->Put("LastFileOpenPath", path);
      prefs->Flush();
    }
  }

  bool GetOpenEditor() const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if(prefs.IsNotNull())
    {
      return prefs->GetBool("OpenEditor", true);
    }
    return true;
  }

  berry::IWorkbenchWindow* m_Window;
};

QmitkFileOpenAction::QmitkFileOpenAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr), d(new QmitkFileOpenActionPrivate)
{
  d->init(window.GetPointer(), this);
}

QmitkFileOpenAction::QmitkFileOpenAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr), d(new QmitkFileOpenActionPrivate)
{
  d->init(window.GetPointer(), this);
  this->setIcon(icon);
}

QmitkFileOpenAction::QmitkFileOpenAction(const QIcon& icon, berry::IWorkbenchWindow* window)
  : QAction(nullptr), d(new QmitkFileOpenActionPrivate)
{
  d->init(window, this);
  this->setIcon(icon);
}

QmitkFileOpenAction::~QmitkFileOpenAction()
{
}

void QmitkFileOpenAction::Run()
{
  auto path = GetPathOfFirstSelectedNode();

  if (path.isEmpty())
    path = d->getLastFileOpenPath();

  // Ask the user for a list of files to open
  QStringList fileNames = QFileDialog::getOpenFileNames(nullptr, "Open",
                                                        path,
                                                        QmitkIOUtil::GetFileOpenFilterString());

  if (fileNames.empty())
    return;

  d->setLastFileOpenPath(fileNames.front());
  mitk::WorkbenchUtil::LoadFiles(fileNames, berry::IWorkbenchWindow::Pointer(d->m_Window),
                                 d->GetOpenEditor());
}
