/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryShell.h"

#include "berryQtDnDControlWidget.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

namespace berry {

QtDnDControlWidget::QtDnDControlWidget(QWidget* parent)
  : QtControlWidget(parent, 0)
{
}

void QtDnDControlWidget::SetTransferTypes(const QStringList& types)
{
  transferTypes = types;
  if (types.isEmpty())
  {
    this->setAcceptDrops(false);
  }
  else
  {
    this->setAcceptDrops(true);
  }
}

void QtDnDControlWidget::AddDropListener(IDropTargetListener* listener)
{
  dndEvents.AddListener(listener);
}

void QtDnDControlWidget::RemoveDropListener(IDropTargetListener* listener)
{
  dndEvents.RemoveListener(listener);
}

void QtDnDControlWidget::dragEnterEvent(QDragEnterEvent* event)
{
  foreach (QString type, transferTypes)
  {
    if (event->mimeData()->hasFormat(type))
    {
      event->acceptProposedAction();
      dndEvents.dragEnter(event);
      break;
    }
  }
}

void QtDnDControlWidget::dragMoveEvent(QDragMoveEvent* event)
{
  dndEvents.dragMove(event);
}

void QtDnDControlWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
  dndEvents.dragLeave(event);
}

void QtDnDControlWidget::dropEvent(QDropEvent* event)
{
  dndEvents.drop(event);
}

}
