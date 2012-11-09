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
#include "mitkWiiMoteIREvent.h"

mitk::WiiMoteIREvent::WiiMoteIREvent(
  mitk::Vector2D inputData
, double recordTime
, int sliceNavigationValue )
: Event(NULL, mitk::Type_WiiMoteInput, mitk::BS_NoButton, mitk::BS_NoButton, Key_none)
{
 this->m_MovementVector = inputData;
 this->m_RecordTime = recordTime;
 this->m_SliceNavigationValue = sliceNavigationValue;
}

mitk::WiiMoteIREvent::~WiiMoteIREvent()
{
}

mitk::Vector2D mitk::WiiMoteIREvent::GetMovementVector() const
{
  return m_MovementVector;
}

double mitk::WiiMoteIREvent::GetRecordTime() const
{
  return m_RecordTime;
}

int mitk::WiiMoteIREvent::GetSliceNavigationValue() const
{
  return m_SliceNavigationValue;
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
  return new Self( this->GetMovementVector()
      , this->GetRecordTime()
      , this->GetSliceNavigationValue() );
}
