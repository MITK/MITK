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
  mitk::StateEvent* se = new mitk::StateEvent(mitk::EIDWIIMOTEINPUT, (const mitk::WiiMoteEvent*)&e);
  this->ForwardEvent(se);
  delete se;
}

void mitk::WiiMoteAddOn::ForwardEvent(const mitk::StateEvent *e)
{
  mitk::GlobalInteraction::GetInstance()->HandleEvent(e);
}




