/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDnDFrameWidget.h>

#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryPlatformUI.h>

#include "internal/QmitkCommonActivator.h"

#include <mitkWorkbenchUtil.h>

#include <QDragEnterEvent>
#include <QMimeData>

class QmitkDnDFrameWidgetPrivate
{
public:

  berry::IPreferences::Pointer GetPreferences() const
  {
    berry::IPreferencesService* prefService = QmitkCommonActivator::GetInstance()->GetPreferencesService();
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
  foreach(QUrl url, fileNames)
  {
    fileNames2.push_back(url.toLocalFile());
  }

  mitk::WorkbenchUtil::LoadFiles(fileNames2,
                                 berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow(),
                                 d->GetOpenEditor());

  event->accept();
}
