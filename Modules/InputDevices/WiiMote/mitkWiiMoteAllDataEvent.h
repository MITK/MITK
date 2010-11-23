#ifndef MITK_WIIMOTE_ALLDATAEVENT_H
#define MITK_WIIMOTE_ALLDATAEVENT_H

// mitk
#include <mitkEvent.h>

// export macro
#include <mitkWiiMoteExports.h>

namespace mitk
{
  /**
  * This event type is used to store all data available from the <br>
  * Wiimote driver. This includes IR, button, and acceleration input.
  */
  class mitkWiiMote_EXPORT WiiMoteAllDataEvent: public Event, itk::EventObject
  {
  public:

    // unnecessary number of parameter because
    // the method MakeObject() from itk::EventObject
    // defines the data, that is able to travel from 
    // thread to thread; i.e. everything, which is not 
    // in the constructor will not be usable in the
    // other thread
    WiiMoteAllDataEvent
      (int eventType, float pitchSpeed, float rollSpeed, float yawSpeed);
    ~WiiMoteAllDataEvent();

    void SetIRDotRawX(float xCoordinate);
    float GetIRDotRawX() const;

    void SetIRDotRawY(float yCoordinate);
    float GetIRDotRawY() const;

    void SetButton(int button);
    int GetButton() const;

    void SetPitchSpeed(float pitchSpeed);
    float GetPitchSpeed() const;

    void SetRollSpeed(float rollSpeed);
    float GetRollSpeed() const;

    void SetYawSpeed(float yawSpeed);
    float GetYawSpeed() const;

    //itk::EventObject implementation
    typedef WiiMoteAllDataEvent Self;
    typedef itk::EventObject Superclass;

    const char * GetEventName() const; 
    bool CheckEvent(const ::itk::EventObject* e) const; 
    ::itk::EventObject* MakeObject() const; 

  protected:

  private:

    // IR data
    // there are in fact 4 registerable dots
    // but at the moment data from one is sufficient
    float m_IRDotRawX;
    float m_IRDotRawY;

    // button data
    int m_Button; // the name of the button

    // acceleration data
    // the unit is degree per second
    float m_PitchSpeed;
    float m_RollSpeed;
    float m_YawSpeed; // only available using MotionPlus

    // orientation data



  }; // end class
} // end namespace
#endif // MITK_WIIMOTE_ALLDATAEVENT_H
