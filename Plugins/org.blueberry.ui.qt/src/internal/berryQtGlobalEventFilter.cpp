/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtGlobalEventFilter.h"
#include "berryShell.h"
#include "internal/berryQtControlWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QWidget>

#include <iostream>

namespace berry {

QtGlobalEventFilter::QtGlobalEventFilter(QObject* parent)
 : QObject(parent)
{

}

bool QtGlobalEventFilter::eventFilter(QObject*  /*obj*/, QEvent* event)
{
  if (event->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    QWidget* widget = QApplication::widgetAt(mouseEvent->globalPos());
    if (widget)
    {
      QObject* parent = widget;
      while (parent)
      {
        if (parent->objectName() == "PartPaneControl")
        {
          (dynamic_cast<QtControlWidget*>(parent))->FireActivateEvent();
          break;
        }
        parent = parent->parent();
      }
    }
  }

  return false;
}

}

