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


#include "QmitkDefaultDropTargetListener.h"
#include <QDebug>
#include <QDropEvent>
#include <QStringList>
#include <QUrl>

#include <mitkWorkbenchUtil.h>
#include <berryPlatformUI.h>

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
  //TODO Qt 4.7 API
  //fileNames2.reserve(fileNames.size());
  foreach(QUrl url, fileNames)
  {
    fileNames2.push_back(url.toLocalFile());
  }

  mitk::WorkbenchUtil::LoadFiles(fileNames2,
                                 berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow());

  event->accept();
}
