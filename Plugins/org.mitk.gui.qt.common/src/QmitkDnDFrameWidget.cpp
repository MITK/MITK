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

#include <QmitkDnDFrameWidget.h>

#include <berryIPreferencesService.h>
#include <berryPlatformUI.h>

#include "internal/QmitkCommonActivator.h"

#include <ctkServiceTracker.h>

#include <mitkWorkbenchUtil.h>

#include <QDragEnterEvent>


class QmitkDnDFrameWidgetPrivate
{
public:

  QmitkDnDFrameWidgetPrivate()
  : m_PrefServiceTracker(QmitkCommonActivator::GetContext())
  {
    m_PrefServiceTracker.open();
  }

  berry::IPreferences::Pointer GetPreferences() const
  {
    berry::IPreferencesService* prefService = m_PrefServiceTracker.getService();
    if (prefService)
    {
      return prefService->GetSystemPreferences()->Node("/General");
    }
    return berry::IPreferences::Pointer(0);
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

  ctkServiceTracker<berry::IPreferencesService*> m_PrefServiceTracker;
};

QmitkDnDFrameWidget::QmitkDnDFrameWidget(QWidget *parent)
: QWidget(parent), d(new QmitkDnDFrameWidgetPrivate())
{
  setAcceptDrops(true);
}

QmitkDnDFrameWidget::~QmitkDnDFrameWidget()
{
}

void QmitkDnDFrameWidget::dragEnterEvent( QDragEnterEvent *event )
{   // accept drags
  event->acceptProposedAction();
}

void QmitkDnDFrameWidget::dropEvent( QDropEvent * event )
{ //open dragged files

  QList<QUrl> fileNames = event->mimeData()->urls();
  if (fileNames.empty())
    return;

  QStringList fileNames2;
  //TODO Qt 4.7 API
  //fileNames2.reserve(fileNames.size());
  foreach(QUrl url, fileNames)
  {
    fileNames2.push_back(url.toLocalFile());
  }

  mitk::WorkbenchUtil::LoadFiles(fileNames2,
                                 berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow(),
                                 d->GetOpenEditor());

  event->accept();
}
