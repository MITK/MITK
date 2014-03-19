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

#ifndef QMITKTCROSSWIDGET_H_INCLUDED
#define QMITKTCROSSWIDGET_H_INCLUDED

#include <QLabel>
#include "MitkQtWidgetsExtExports.h"

#include <mitkCommon.h>

class MitkQtWidgetsExt_EXPORT QmitkCrossWidget : public QLabel
{

  Q_OBJECT

public:

  QmitkCrossWidget( QWidget * parent=0, Qt::WindowFlags f = 0 );

  void mousePressEvent( QMouseEvent* mouseEvent );
  void mouseMoveEvent( QMouseEvent* mouseEvent );
  void mouseReleaseEvent( QMouseEvent* mouseEvent );

 signals:

  void SignalDeltaMove( int,int );

protected:

 // fix for bug 3378 - setPos() causes an app crash on MAC OS X
#ifdef __APPLE__
  void ResetMousePosition(int, int) {};
#else
  void ResetMousePosition(int xpos, int ypos)
  {
      QCursor::setPos(xpos, ypos);
  };
#endif

  int lastX,lastY;

};

#endif

