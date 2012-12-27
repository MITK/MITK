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
#include "mitkWiiMoteButtonEvent.h"

mitk::WiiMoteButtonEvent::WiiMoteButtonEvent(int type, int button, int buttonState, int key)
: Event(NULL, type, button, buttonState, key)
{
}

mitk::WiiMoteButtonEvent::~WiiMoteButtonEvent()
{
}

const char* mitk::WiiMoteButtonEvent::GetEventName() const
{
  return "WiiMoteButtonEvent";
}

bool mitk::WiiMoteButtonEvent::CheckEvent(const itk::EventObject *e) const
{
  return dynamic_cast<const Self*>(e);
}

itk::EventObject* mitk::WiiMoteButtonEvent::MakeObject() const
{
    return new Self(m_Type, m_Button, m_ButtonState, m_Key);
}
