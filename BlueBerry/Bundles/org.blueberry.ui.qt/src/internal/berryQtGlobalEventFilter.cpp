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

#include "berryQtGlobalEventFilter.h"
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

