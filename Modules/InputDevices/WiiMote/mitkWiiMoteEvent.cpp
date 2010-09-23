#include "mitkWiiMoteEvent.h"

mitk::WiiMoteEvent::WiiMoteEvent(mitk::Vector2D inputData, double recordTime) 
: Event(NULL, mitk::Type_WiiMoteInput, mitk::BS_NoButton, mitk::BS_NoButton, Key_none)
{
 this->m_MovementVector = inputData;
 this->m_RecordTime = recordTime;
}

// TODO: generic parameter for Type
mitk::WiiMoteEvent::WiiMoteEvent(mitk::EEventType eventType) 
: Event(NULL, eventType, mitk::BS_NoButton, mitk::BS_NoButton, Key_none)
, m_RecordTime(NULL)
{
  m_EventType = eventType;
  m_MovementVector.Fill(0);
}

mitk::WiiMoteEvent::~WiiMoteEvent() 
{
}

mitk::Vector2D mitk::WiiMoteEvent::GetMovementVector() const
{
  return m_MovementVector;
}

double mitk::WiiMoteEvent::GetRecordTime()
{
  return m_RecordTime;
}

const char* mitk::WiiMoteEvent::GetEventName() const
{ 
  return "WiiMoteEvent"; 
} 

bool mitk::WiiMoteEvent::CheckEvent(const itk::EventObject *e) const
{ 
  return dynamic_cast<const Self*>(e); 
} 

itk::EventObject* mitk::WiiMoteEvent::MakeObject() const
{ 
  if(m_EventType != mitk::Type_WiiMoteHomeButton)
  {
    return new Self(m_MovementVector, m_RecordTime); 
  }
  else
  {
    return new Self(m_EventType);
  }
} 
