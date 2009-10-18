/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryQtGlobalEventFilter.h"
#include "../internal/cherryQtControlWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QWidget>

#include <iostream>

namespace cherry {

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

