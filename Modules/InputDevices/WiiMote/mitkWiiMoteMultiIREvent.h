#ifndef MITK_WIIMOTEMULTIIREVENT_H
#define MITK_WIIMOTEMULITIREVENT_H

#include "mitkInputDevicesExports.h"

#include "mitkEvent.h"
#include "mitkVector.h"
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