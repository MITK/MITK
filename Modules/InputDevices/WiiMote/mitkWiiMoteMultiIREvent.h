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
#ifndef MITK_WIIMOTEMULTIIREVENT_H
#define MITK_WIIMOTEMULITIREVENT_H

#include <MitkInputDevicesExports.h>

#include "mitkEvent.h"
#include "mitkNumericTypes.h"
#include "mitkInteractionConst.h"

namespace mitk
{
  class mitkInputDevices_EXPORT WiiMoteMultiIREvent : public Event, itk::EventObject
  {
  public:

    typedef WiiMoteMultiIREvent Self;
    typedef itk::EventObject Superclass;

    WiiMoteMultiIREvent(mitk::Point3D Coordinate3D);
    ~WiiMoteMultiIREvent();

    mitk::Point3D Get3DCoordinate() const;

    //itk::EventObject implementation
    const char * GetEventName() const;
    bool CheckEvent(const ::itk::EventObject* e) const;
    ::itk::EventObject* MakeObject() const;

  protected:

  private:

    mitk::Point3D m_3DCoordinate;

  }; // end class
} // end namspace

#endif  // MITK_WIIMOTEMULITIREVENT_H
