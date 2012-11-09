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


#include "mitkStateEvent.h"
#include "mitkEvent.h"

mitk::StateEvent::StateEvent(int id, Event const* event)
:m_Id(id), m_Event(event)
{
}

mitk::StateEvent::StateEvent()
:m_Id(0), m_Event(0)
{
}

mitk::StateEvent::~StateEvent()
{
}

void mitk::StateEvent::Set(int id, Event const* event)
{
  m_Id = id;
  m_Event = event;
}

int mitk::StateEvent::GetId() const
{
  return m_Id;
}

mitk::Event const* mitk::StateEvent::GetEvent() const
{
  return m_Event;
}
