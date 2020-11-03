/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryShell.h"

#include "berryQtDnDControlWidget.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

namespace berry {

QtDnDControlWidget::QtDnDControlWidget(QWidget* parent)
  : QtControlWidget(parent, nullptr)
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
