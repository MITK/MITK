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

#ifndef QMITKEVENTADAPTER_H_
#define QMITKEVENTADAPTER_H_

#include <QmitkExports.h>

#include <QMouseEvent>
#include <QKeyEvent>
#include <mitkKeyEvent.h>
#include <mitkWheelEvent.h>
#include <mitkDisplayPositionEvent.h>

class QMITK_EXPORT QmitkEventAdapter
{
public:

  static mitk::MouseEvent AdaptMouseEvent(mitk::BaseRenderer* sender, QMouseEvent* mouseEvent);
  static mitk::WheelEvent AdaptWheelEvent(mitk::BaseRenderer* sender, QWheelEvent* wheelEvent);
  static mitk::KeyEvent AdaptKeyEvent(mitk::BaseRenderer* sender, QKeyEvent* keyEvent, const QPoint& point);
};

#endif /*QMITKEVENTADAPTER_H_*/
