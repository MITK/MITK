/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCrosshairPositionEvent_h
#define mitkCrosshairPositionEvent_h

#include <MitkCoreExports.h>
#include <mitkInteractionEvent.h>

namespace mitk
{
  /** A special mitk::Event thrown by the SliceNavigationController on mouse scroll
    */
  class MITKCORE_EXPORT CrosshairPositionEvent : public mitk::InteractionEvent
  {
  public:
    CrosshairPositionEvent(BaseRenderer *sender);
  };
}

#endif
