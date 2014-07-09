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

#include <QmitkIOUtil.h>

#include <QFileDialog>

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
  QStringList fileNames =  QFileDialog::getOpenFileNames(NULL, "Open",
                                                         d->getLastFileOpenPath(),
                                                         QmitkIOUtil::GetFileOpenFilterString());

  if (fileNames.empty())
    return;

  d->setLastFileOpenPath(fileNames.front());

  mitk::WorkbenchUtil::LoadFiles(fileNames, d->m_Window.Lock(), d->GetOpenEditor());
}
