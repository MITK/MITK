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
#include "mitkWiiMoteCalibrationEvent.h"

mitk::WiiMoteCalibrationEvent::WiiMoteCalibrationEvent(double rawX, double rawY)
: Event(NULL, mitk::Type_WiiMoteInput, mitk::BS_NoButton, mitk::BS_NoButton, Key_none)
{
  m_RawX = rawX;
  m_RawY = rawY;
}

mitk::WiiMoteCalibrationEvent::~WiiMoteCalibrationEvent()
{
}

double mitk::WiiMoteCalibrationEvent::GetXCoordinate() const
{
  return m_RawX;
}

double mitk::WiiMoteCalibrationEvent::GetYCoordinate() const
{
  return m_RawY;
}

const char* mitk::WiiMoteCalibrationEvent::GetEventName() const
{
  return "WiiMoteCalibrationEvent";
}

bool mitk::WiiMoteCalibrationEvent::CheckEvent(const itk::EventObject *e) const
{
  return dynamic_cast<const Self*>(e);
}

itk::EventObject* mitk::WiiMoteCalibrationEvent::MakeObject() const
{
    return new Self(m_RawX, m_RawY);
}
