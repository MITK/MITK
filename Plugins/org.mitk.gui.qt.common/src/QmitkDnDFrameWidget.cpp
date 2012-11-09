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

#include <berryPlatformUI.h>

#include "mitkWorkbenchUtil.h"

#include <QDragEnterEvent>


QmitkDnDFrameWidget::QmitkDnDFrameWidget(QWidget *parent)
: QWidget(parent)
{
  setAcceptDrops(true);
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
                                 berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow());

  event->accept();
}
