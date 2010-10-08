#include "mitkWiiMoteAddOn.h"

#include "mitkGlobalInteraction.h"
#include "mitkInteractionConst.h"

#include "mitkWiiMoteThread.h"

mitk::WiiMoteAddOn* mitk::WiiMoteAddOn::GetInstance()
{
  static WiiMoteAddOn instance;
  return &instance;
}

mitk::WiiMoteAddOn::WiiMoteAddOn(): m_WiiMoteThread(NULL)
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
  // apparently the dynamic cast does not work here
  // reason unknown - the normal cast however works fine
  mitk::StateEvent* se = new mitk::StateEvent(mitk::EIDWIIMOTEINPUT, (const mitk::WiiMoteIREvent*)&e);
  this->ForwardEvent(se);
  delete se;
}

void mitk::WiiMoteAddOn::WiiMoteButtonPressed(const itk::EventObject &e)
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

void mitk::WiiMoteAddOn::WiiMoteCalibrationInput(const itk::EventObject &e)
{  
  mitk::StateEvent* se = new mitk::StateEvent(mitk::EIDWIIMOTEINPUT, (const mitk::WiiMoteCalibrationEvent*)&e);
  this->ForwardEvent(se);
  delete se;
}

void mitk::WiiMoteAddOn::ForwardEvent(const mitk::StateEvent *e)
{
  mitk::GlobalInteraction::GetInstance()->HandleEvent(e);
}




