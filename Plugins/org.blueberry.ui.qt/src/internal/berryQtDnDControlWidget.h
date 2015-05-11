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


#ifndef BERRYQTDNDCONTROLWIDGET_H
#define BERRYQTDNDCONTROLWIDGET_H

#include "berryQtControlWidget.h"

#include "berryIDropTargetListener.h"

#include <QMimeData>

namespace berry {

class QtDnDControlWidget : public QtControlWidget
{
public:
  QtDnDControlWidget(QWidget *parent = nullptr);

  void SetTransferTypes(const QStringList& types);

  void AddDropListener(IDropTargetListener* listener);
  void RemoveDropListener(IDropTargetListener* listener);

protected:

  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

private:

  IDropTargetListener::Events dndEvents;
  QStringList transferTypes;
};

}

#endif // BERRYQTDNDCONTROLWIDGET_H
