/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDnDFrameWidget.h>

#include <berryPlatformUI.h>

#include "internal/QmitkCommonActivator.h"

#include <mitkWorkbenchUtil.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <QDragEnterEvent>
#include <QMimeData>

class QmitkDnDFrameWidgetPrivate
{
public:

  mitk::IPreferences* GetPreferences() const
  {
    auto* prefService = QmitkCommonActivator::GetInstance()->GetPreferencesService();

    return prefService != nullptr
      ? prefService->GetSystemPreferences()->Node("/General")
      : nullptr;
  }

  bool GetOpenEditor() const
  {
    auto* prefs = GetPreferences();
    if(prefs != nullptr)
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
