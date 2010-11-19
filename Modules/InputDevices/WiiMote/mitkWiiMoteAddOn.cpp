#include "mitkWiiMoteAddOn.h"

#include "mitkGlobalInteraction.h"
#include "mitkInteractionConst.h"

#include "mitkWiiMoteThread.h"
#include "mitkWiiMoteIREvent.h"
#include "mitkWiiMoteButtonEvent.h"
#include "mitkWiiMoteCalibrationEvent.h"
#include <mitkWiiMoteAllDataEvent.h>

mitk::WiiMoteAddOn* mitk::WiiMoteAddOn::GetInstance()
{
  static WiiMoteAddOn instance;
  return &instance;
}

mitk::WiiMoteAddOn::WiiMoteAddOn()
: m_WiiMoteThread(NULL)
{
}

mitk::WiiMoteAddOn::~WiiMoteAddOn()
{
  this->DeactivateWiiMotes();
}

void mitk::WiiMoteAddOn::ActivateWiiMotes()
{
  if(m_WiiMoteThread == NULL)
  {
    m_WiiMoteThread = new WiiMoteThread();
    m_WiiMoteThread->Run();
  }
}

void mitk::WiiMoteAddOn::DeactivateWiiMotes()
{
  if(m_WiiMoteThread != NULL)
  {
    m_WiiMoteThread->StopWiiMote();

    //needed, otherwise the mutex wants to unlock
    //when the object is already destroyed
    Sleep(1000);

    m_WiiMoteThread->Delete();
    m_WiiMoteThread = NULL;
  }
}

void mitk::WiiMoteAddOn::WiiMoteInput(const itk::EventObject& e)
{
  try
  {
    // apparently the dynamic cast does not work here
    // reason unknown - the normal cast however works fine
    mitk::StateEvent* se = new mitk::StateEvent(mitk::EIDWIIMOTEINPUT, (const mitk::WiiMoteIREvent*)&e);
    this->ForwardEvent(se);
    delete se;
  }
  catch(...)
  {
    MITK_ERROR << "Wiimote event for headtracking IR input could not be transformed";
  }
}

void mitk::WiiMoteAddOn::WiiMoteButtonPressed(const itk::EventObject &e)
{
  try
  {
    mitk::WiiMoteButtonEvent const* wiiEvent = (const mitk::WiiMoteButtonEvent*)(&e);
    int key = wiiEvent->GetKey();
    mitk::StateEvent* se;

    switch(key)
    {
    case mitk::Key_Home:
      se = new mitk::StateEvent(mitk::EIDWIIMOTEBUTTON, wiiEvent);
      break;
    case mitk::Key_A:
      se = new mitk::StateEvent(mitk::EV_INIT, wiiEvent);
      break;
    }

    if(se != NULL)
    {
      this->ForwardEvent(se);
      delete se;
    }
  }
  catch(...)
  {
    MITK_ERROR << "Wiimote event for headtracking button pressed could not be transformed";
  }
}

void mitk::WiiMoteAddOn::WiiMoteCalibrationInput(const itk::EventObject &e)
{  
  try
  {
  mitk::StateEvent* se = new mitk::StateEvent(mitk::EIDWIIMOTEINPUT, (const mitk::WiiMoteCalibrationEvent*)&e);
  this->ForwardEvent(se);
  delete se;
  }
  catch(...)
  {
    MITK_ERROR << "Wiimote event for headtracking calibration input could not be transformed";
  }
}

void mitk::WiiMoteAddOn::SetWiiMoteSurfaceIModus(bool activated)
{
  m_WiiMoteThread->SetWiiMoteSurfaceIModus(activated);
}

void mitk::WiiMoteAddOn::WiiMoteSurfaceInteractionInput(const itk::EventObject& e)
{
  try
  {
    mitk::StateEvent* se = new mitk::StateEvent(mitk::EIDWIIMOTEINPUT, (const mitk::WiiMoteAllDataEvent*)&e);
    this->ForwardEvent(se);
    delete se;
  }
  catch(...)
  {
    MITK_ERROR << "Wiimote event for surface interaction could not be transformed";
  }
}





