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


#include "mitkEvent.h"

mitk::Event::Event(mitk::BaseRenderer* sender, int type, int button,
                   int buttonState, int key)
: m_Sender( sender ), m_Type( type ), m_Button( button ), m_ButtonState( buttonState ),
  m_Key( key )
{
}

mitk::Event::~Event()
{
}

mitk::BaseRenderer* mitk::Event::GetSender() const
{
  return m_Sender;
}

int mitk::Event::GetType() const
{
  return m_Type;
}

int mitk::Event::GetButtonState() const
{
  return m_ButtonState;
}

int mitk::Event::GetButton() const
{
  return m_Button;
}


int mitk::Event::GetKey() const
{
  return m_Key;
}

bool mitk::Event::operator==(const Event& event) const
{
  const mitk::Event *input = dynamic_cast<const mitk::Event*>(&event);
  if(input==nullptr) return false;

  if ( (m_Type == event.GetType())
    && (m_Button == event.GetButton())
    && (m_ButtonState == event.GetButtonState())
    && (m_Key == event.GetKey()) )
  {
    return true;
  }
  else
  {
    return false;
  }
}
