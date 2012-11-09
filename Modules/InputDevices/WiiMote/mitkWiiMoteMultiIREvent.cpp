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
#include "mitkWiiMoteMultiIREvent.h"

mitk::WiiMoteMultiIREvent::WiiMoteMultiIREvent(mitk::Point3D Coordinate3D)
: Event(NULL, mitk::Type_WiiMoteInput, mitk::BS_NoButton, mitk::BS_NoButton, Key_none)
{
  m_3DCoordinate = Coordinate3D;
}

mitk::WiiMoteMultiIREvent::~WiiMoteMultiIREvent()
{
}

mitk::Point3D mitk::WiiMoteMultiIREvent::Get3DCoordinate() const
{
  return m_3DCoordinate;
}


const char* mitk::WiiMoteMultiIREvent::GetEventName() const
{
  return "WiiMoteMultiIREvent";
}

bool mitk::WiiMoteMultiIREvent::CheckEvent(const itk::EventObject *e) const
{
  return dynamic_cast<const Self*>(e);
}

itk::EventObject* mitk::WiiMoteMultiIREvent::MakeObject() const
{
    return new Self(m_3DCoordinate);
}
