#include "mitkWiiMoteIREvent.h"

mitk::WiiMoteIREvent::WiiMoteIREvent(mitk::Vector2D inputData, double recordTime) 
: Event(NULL, mitk::Type_WiiMoteInput, mitk::BS_NoButton, mitk::BS_NoButton, Key_none)
{
 this->m_MovementVector = inputData;
 this->m_RecordTime = recordTime;
}

mitk::WiiMoteIREvent::~WiiMoteIREvent() 
{
}

mitk::Vector2D mitk::WiiMoteIREvent::GetMovementVector() const
{
  return m_MovementVector;
}

double mitk::WiiMoteIREvent::GetRecordTime()
{
  return m_RecordTime;
}

const char* mitk::WiiMoteIREvent::GetEventName() const
{ 
  return "WiiMoteIREvent"; 
} 

bool mitk::WiiMoteIREvent::CheckEvent(const itk::EventObject *e) const
{ 
  return dynamic_cast<const Self*>(e); 
} 

itk::EventObject* mitk::WiiMoteIREvent::MakeObject() const
{ 
    return new Self(m_MovementVector, m_RecordTime); 
} 
