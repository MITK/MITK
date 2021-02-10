/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDnDDataNodeWidget.h>

// mitk qt widgets module
#include "QmitkMimeTypes.h"

// qt
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

QmitkDnDDataNodeWidget::QmitkDnDDataNodeWidget(QWidget* parent /*= nullptr*/)
  : QFrame(parent)
{
  this->setAcceptDrops(true);
}

QmitkDnDDataNodeWidget::~QmitkDnDDataNodeWidget()
{
  // nothing here
}

void QmitkDnDDataNodeWidget::dragEnterEvent(QDragEnterEvent* event)
{
  if (this != event->source())
  {
    event->acceptProposedAction();
  }
  else
  {
    event->ignore();
  }
}

void QmitkDnDDataNodeWidget::dragMoveEvent(QDragMoveEvent* event)
{
  if (event->mimeData()->hasFormat(QmitkMimeTypes::DataNodePtrs))
  {
    event->acceptProposedAction();
  }
  else
  {
    event->ignore();
  }
}

void QmitkDnDDataNodeWidget::dropEvent(QDropEvent* event)
{
  QList<mitk::DataNode*> dataNodeList = QmitkMimeTypes::ToDataNodePtrList(event->mimeData());
  if (!dataNodeList.empty())
  {
    emit NodesDropped(dataNodeList.toVector().toStdVector());
  }

  event->acceptProposedAction();
}

