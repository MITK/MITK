/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
