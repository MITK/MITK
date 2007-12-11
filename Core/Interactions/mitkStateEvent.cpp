/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkStateEvent.h"
#include "mitkEvent.h"

//##ModelId=3F02F8960177
mitk::StateEvent::StateEvent(int id, Event const* event)
:m_Id(id), m_Event(event)
{
}
//##ModelId=3E5B7B9E0137
mitk::StateEvent::StateEvent()
:m_Id(0), m_Event(0)
{
}

//##ModelId=3F02F89601A5
mitk::StateEvent::~StateEvent()
{
}

//##ModelId=3E5B7A7603DA
void mitk::StateEvent::Set(int id, Event const* event)
{
  m_Id = id;
  m_Event = event;
}

//##ModelId=3E5B7AEC0394
int mitk::StateEvent::GetId() const
{
  return m_Id;
}

//##ModelId=3E5B7B030383
mitk::Event const* mitk::StateEvent::GetEvent() const
{
  return m_Event;
}
