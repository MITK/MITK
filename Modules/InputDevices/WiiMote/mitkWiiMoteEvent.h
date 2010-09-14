#ifndef MITK_WIIMOTEEVENT_H
#define MITK_WIIMOTEEVENT_H

#include "mitkEvent.h"
#include "mitkVector.h"

#include "mitkInputDevicesExports.h"

namespace mitk
{
  /**
  * This event type is used for any events triggered by the <br>
  * the Wiimote driver. It allows to process the given information <br>
  * (e.g. sensor input, button triggering) fired by the Wiimote.
  */
  class mitkInputDevices_EXPORT WiiMoteEvent : public Event, itk::EventObject
  {

  public:

    typedef WiiMoteEvent Self; 
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
    WiiMoteEvent(mitk::Vector2D inputData, double recordTime);
    
    /**
    * @brief default destructor
    **/
    ~WiiMoteEvent();

    mitk::Vector2D GetMovementVector() const;
    double GetRecordTime();

    //itk::EventObject implementation
    virtual const char * GetEventName() const; 
    virtual bool CheckEvent(const ::itk::EventObject* e) const; 
    virtual ::itk::EventObject* MakeObject() const; 


  private:

    mitk::Vector2D m_MovementVector;
    double m_RecordTime;

  }; // end class
} // end namespace mitk
#endif // MITK_WIIMOTEEVENT_H