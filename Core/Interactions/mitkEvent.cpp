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


#include "mitkEvent.h"

//##ModelId=3E5B3007000F
mitk::Event::Event(mitk::BaseRenderer* sender, int type, int button,
                   int buttonState, int key)
: m_Sender( sender ), m_Type( type ), m_Button( button ), m_ButtonState( buttonState ),
  m_Key( key )
{
}

//##ModelId=3EF099E8023A
mitk::Event::~Event()
{
}

//##ModelId=3EF1D2A00139
mitk::BaseRenderer* mitk::Event::GetSender() const
{
  return m_Sender;
}

//##ModelId=3E5B304700A7
int mitk::Event::GetType() const
{
  return m_Type;
}

//##ModelId=3E5B3055015C
int mitk::Event::GetButtonState() const
{
  return m_ButtonState;
}

//##ModelId=3E8AE2F20360
int mitk::Event::GetButton() const
{
  return m_Button;
}


//##ModelId=3E5B306F0221
int mitk::Event::GetKey() const
{
  return m_Key;
}

//##ModelId=3E77630102A1
bool mitk::Event::operator==(const Event& event) const
{
  const mitk::Event *input = dynamic_cast<const mitk::Event*>(&event);
  if(input==NULL) return false;

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
