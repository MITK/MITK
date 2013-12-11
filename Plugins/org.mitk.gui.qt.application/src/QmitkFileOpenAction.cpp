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

#include <mitkWorkbenchUtil.h>
#include <mitkCoreObjectFactory.h>

#include <QFileDialog>
#include <QFileInfo>

#include <ctkServiceTracker.h>

#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbench.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>
#include <berryUIException.h>
#include <QMessageBox.h>
#include <mitkSceneIO.h>

class QmitkFileOpenActionPrivate
{
public:

  void init ( berry::IWorkbenchWindow::Pointer window, QmitkFileOpenAction* action )
  {
    m_Window = window;
    action->setParent(static_cast<QWidget*>(m_Window.Lock()->GetShell()->GetControl()));
    action->setText("&Open...");
    action->setToolTip("Open data files (images, surfaces,...)");

    QObject::connect(action, SIGNAL(triggered(bool)), action, SLOT(Run()));
  }

  berry::IPreferences::Pointer GetPreferences() const
  {
    berry::IPreferencesService::Pointer prefService = mitk::PluginActivator::GetInstance()->GetPreferencesService();
    if (prefService.IsNotNull())
    {
      return prefService->GetSystemPreferences()->Node("/General");
    }
    return berry::IPreferences::Pointer(0);
  }

  QString getLastFileOpenPath() const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if(prefs.IsNotNull())
    {
      return QString::fromStdString(prefs->Get("LastFileOpenPath", ""));
    }
    return QString();
  }

  void setLastFileOpenPath(const QString& path) const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if(prefs.IsNotNull())
    {
      prefs->Put("LastFileOpenPath", path.toStdString());
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

  berry::IWorkbenchWindow::WeakPtr m_Window;
};

QmitkFileOpenAction::QmitkFileOpenAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(0), d(new QmitkFileOpenActionPrivate)
{
  d->init(window, this);
}

QmitkFileOpenAction::QmitkFileOpenAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
  : QAction(0), d(new QmitkFileOpenActionPrivate)
{
  d->init(window, this);
  this->setIcon(icon);
}

QmitkFileOpenAction::~QmitkFileOpenAction()
{
}

void QmitkFileOpenAction::Run()
{

  // Ask the user for a list of files to open
  QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Open",
                                                        d->getLastFileOpenPath(),
                                                        mitk::CoreObjectFactory::GetInstance()->GetFileExtensions());

  if (fileNames.empty())
    return;

  d->setLastFileOpenPath(fileNames.front());

  // muellerm, 11.12.13: added logic for persistent data inclusion
  bool sceneFileLoad = false;
  for( int i=0; i<fileNames.size(); ++i)
  {
      QFileInfo fileInfo( fileNames.at(i) );
      if( fileInfo.suffix() == "mitk" )
      {
          sceneFileLoad = true;
          break;
      }
  }

  if(sceneFileLoad)
  {
      berry::IPreferencesService::Pointer prefService
          = berry::Platform::GetServiceRegistry()
          .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
      berry::IPreferences::Pointer prefs = prefService->GetSystemPreferences()->Node("/General");
      bool loadPersistentDataWithScene = prefs->GetBool("loadPersistentDataWithScene", false);
      mitk::SceneIO::SetLoadPersistentDataWithScene(loadPersistentDataWithScene);
      bool loadPersistentDataWithSceneUserAlreadyAsked = prefs->GetBool("loadPersistentDataWithSceneUserAlreadyAsked", false);

      if( !loadPersistentDataWithSceneUserAlreadyAsked )
      {
          int answer = QMessageBox::question( NULL, "Load additional application data?", "<html>Load additional application data from Scene file?<br />" + QString("<strong>Help:</strong>&nbsp;Modules and plugins of the MITK Workbench can store data in an internal database. This database can be included into scene files while saving or restored while loading a scene file. This is useful if the modules you are using support this internal database and you want to save images along with application data (e.g. settings, parameters, etc.).")
              + QString::fromStdString("<br />Your answer will be saved and you will not be asked again. You can change this behavior later in the General Preferences Page.</html>"),
              QMessageBox::Yes,
              QMessageBox::No );

          if( answer == QMessageBox::No )
          {
              loadPersistentDataWithScene = false;
          }
          else
          {
              loadPersistentDataWithScene = true;

          }
          loadPersistentDataWithSceneUserAlreadyAsked = true;
          prefs->PutBool("loadPersistentDataWithSceneUserAlreadyAsked", loadPersistentDataWithSceneUserAlreadyAsked);
          prefs->PutBool("loadPersistentDataWithScene", loadPersistentDataWithScene);
          prefs->Flush();
      }
      // muellerm, 11.12.13: end of changes
  }

  mitk::WorkbenchUtil::LoadFiles(fileNames, d->m_Window.Lock(), d->GetOpenEditor());
}
