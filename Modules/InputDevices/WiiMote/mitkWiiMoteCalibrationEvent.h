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
#ifndef MITK_WIIMOTECALIBRATIONEVENT_H
#define MITK_WIIMOTECALIBRATIONEVENT_H

#include <MitkWiiMoteExports.h>

#include "mitkEvent.h"
#include "mitkNumericTypes.h"
#include "mitkInteractionConst.h"

namespace mitk
{
  class mitkWiiMote_EXPORT WiiMoteCalibrationEvent : public Event, itk::EventObject
  {

  public:

    typedef WiiMoteCalibrationEvent Self;
    typedef itk::EventObject Superclass;

    /**
    * Initializes a Wiimote Event, that stores additional information. <br>
    * Such as a the raw x and y coordinates of the IR input. <br>
    *
    * @param rawX
    *          x coordinate of the IR sensor input
    * @param rawY
    *          y coordinate of the IR sensor input
    */
    WiiMoteCalibrationEvent(double rawX, double rawY);
    ~WiiMoteCalibrationEvent();

    double GetXCoordinate() const;
    double GetYCoordinate() const;

    //itk::EventObject implementation
    const char * GetEventName() const;
    bool CheckEvent(const ::itk::EventObject* e) const;
    ::itk::EventObject* MakeObject() const;

  protected:

  private:

    double m_RawX;
    double m_RawY;

  };
}


#endif // MITK_WIIMOTECALIBRATIONEVENT_H
