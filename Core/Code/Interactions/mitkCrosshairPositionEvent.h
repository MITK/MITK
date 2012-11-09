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

#ifndef mitkCrosshairPositionEvent_h
#define mitkCrosshairPositionEvent_h

#include <MitkExports.h>
#include "mitkEvent.h"

namespace mitk {

/** A special mitk::Event thrown by the SliceNavigationController on mouse scroll
  */
class MITK_CORE_EXPORT CrosshairPositionEvent : public Event
{
  public:
    CrosshairPositionEvent(BaseRenderer* sender);
};

}

#endif
