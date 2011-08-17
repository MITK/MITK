/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef mitkCrosshairPositionEvent_h
#define mitkCrosshairPositionEvent_h

#include "mitkCommon.h"
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
