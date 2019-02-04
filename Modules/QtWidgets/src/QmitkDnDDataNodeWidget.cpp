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

#include <QmitkDnDDataNodeWidget.h>

// mitk qt widgets module
#include "QmitkMimeTypes.h"

// qt
#include <QMimeData>

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

