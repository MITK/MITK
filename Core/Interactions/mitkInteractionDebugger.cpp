#include "mitkInteractionDebugger.h"
#include <itkmacro.h>

const char* mitk::InteractionDebugger::m_CurrentSender = "";
bool mitk::InteractionDebugger::m_Active = false;

mitk::InteractionDebugger::InteractionDebugger()
{}

void mitk::InteractionDebugger::Activate()
{
  m_Active = true;
}

void mitk::InteractionDebugger::Deactivate()
{
  m_Active = false;
}
mitk::InteractionDebugger::~InteractionDebugger()
{}

void mitk::InteractionDebugger::Set(const char* sender, const char* text)
{
  if (m_Active)
  {
    itk::OStringStream itkmsg;
    if (! itk::Object::GetGlobalWarningDisplay())
      return;

    if (sender != m_CurrentSender)
    {
      itkmsg << sender <<" :\n"<<text<<"\n";
      m_CurrentSender = sender;
    }
    else
      itkmsg<<text<<"\n";
    
    itk::OutputWindowDisplayDebugText(itkmsg.str().c_str());
  }
}

