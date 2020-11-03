/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkDefaultDropTargetListener.h"
#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QStringList>
#include <QUrl>

#include "internal/org_mitk_gui_qt_application_Activator.h"

#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryPlatformUI.h>

#include <mitkWorkbenchUtil.h>

class QmitkDefaultDropTargetListenerPrivate
{
public:

  berry::IPreferences::Pointer GetPreferences() const
  {
    berry::IPreferencesService* prefService = mitk::PluginActivator::GetInstance()->GetPreferencesService();
    if (prefService)
    {
      return prefService->GetSystemPreferences()->Node("/General");
    }
    return berry::IPreferences::Pointer(nullptr);
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
};

QmitkDefaultDropTargetListener::QmitkDefaultDropTargetListener()
  : berry::IDropTargetListener(), d(new QmitkDefaultDropTargetListenerPrivate())
{
}

QmitkDefaultDropTargetListener::~QmitkDefaultDropTargetListener()
{
}

berry::IDropTargetListener::Events::Types QmitkDefaultDropTargetListener::GetDropTargetEventTypes() const
{
  return Events::DROP;
}

void QmitkDefaultDropTargetListener::DropEvent(QDropEvent *event)
{
  qDebug() << event->mimeData()->formats();
  qDebug() << event->mimeData()->text();

  QList<QUrl> fileNames = event->mimeData()->urls();
  if (fileNames.empty())
    return;

  QStringList fileNames2;
  foreach(QUrl url, fileNames)
  {
    fileNames2.push_back(url.toLocalFile());
  }

  mitk::WorkbenchUtil::LoadFiles(fileNames2,
                                 berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow(),
                                 d->GetOpenEditor());

  event->accept();
}
