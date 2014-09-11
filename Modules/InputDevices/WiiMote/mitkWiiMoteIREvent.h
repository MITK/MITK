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
#ifndef MITK_WIIMOTEIREVENT_H
#define MITK_WIIMOTEIREVENT_H

#include "mitkEvent.h"
#include "mitkNumericTypes.h"
#include "mitkInteractionConst.h"

#include <MitkWiiMoteExports.h>

namespace mitk
{
  /**
  * This event type is used for IR events triggered by the <br>
  * the Wiimote driver.
  */
  class mitkWiiMote_EXPORT WiiMoteIREvent : public Event, itk::EventObject
  {

  public:

    typedef WiiMoteIREvent Self;
    typedef itk::EventObject Superclass;

    /**
    * Initializes a Wiimote Event, that stores additional information. <br>
    * Such as a vector and the time of the recording <br>
    *
    * @param inputData
    *          the movement of the IR sensor computed in a vector
    * @param recordTime
    *          the time at which the data was recorded
    */
    WiiMoteIREvent( mitk::Vector2D inputData
      , double recordTime
      , int sliceNavigationValue);

    ~WiiMoteIREvent();

    /**
    * Returns the current movement vector with the coordinates <br>
    * in the following order: x, y, z
    */
    mitk::Vector2D GetMovementVector() const;
    double GetRecordTime() const;
    int GetSliceNavigationValue() const;

    //itk::EventObject implementation
    const char * GetEventName() const;
    bool CheckEvent(const ::itk::EventObject* e) const;
    ::itk::EventObject* MakeObject() const;

  private:

    mitk::Vector2D m_MovementVector;
    double m_RecordTime;
    int m_SliceNavigationValue;

  }; // end class
} // end namespace mitk
#endif // MITK_WIIMOTEIREVENT_H
