/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef BERRYQTDNDCONTROLWIDGET_H
#define BERRYQTDNDCONTROLWIDGET_H

#include "berryQtControlWidget.h"

#include "../berryIDropTargetListener.h"

#include <QMimeData>

namespace berry {

class QtDnDControlWidget : public QtControlWidget
{
public:
  QtDnDControlWidget(QWidget *parent = 0);

  void SetTransferTypes(const QStringList& types);

  void AddDropListener(IDropTargetListener* listener);
  void RemoveDropListener(IDropTargetListener* listener);

protected:

  void dragEnterEvent(QDragEnterEvent* event);
  void dragMoveEvent(QDragMoveEvent* event);
  void dragLeaveEvent(QDragLeaveEvent* event);
  void dropEvent(QDropEvent* event);

private:

  IDropTargetListener::Events dndEvents;
  QStringList transferTypes;
};

}

#endif // BERRYQTDNDCONTROLWIDGET_H
