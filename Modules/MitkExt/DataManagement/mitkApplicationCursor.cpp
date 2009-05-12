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

#include "mitkApplicationCursor.h"

#include <iostream>

mitk::ApplicationCursor* mitk::ApplicationCursor::m_Instance = NULL;
mitk::ApplicationCursorImplementation* mitk::ApplicationCursor::m_Implementation = NULL;

namespace mitk {

ApplicationCursor::ApplicationCursor()
{
}

ApplicationCursor* ApplicationCursor::GetInstance()
{
  if (!m_Instance)
  {
    m_Instance = new ApplicationCursor();
  }

  return m_Instance;
}
    
void ApplicationCursor::RegisterImplementation(ApplicationCursorImplementation* implementation)
{
  m_Implementation = implementation;
}

void ApplicationCursor::PushCursor(const char* XPM[], int hotspotX, int hotspotY)
{
  if (m_Implementation)
  {
    m_Implementation->PushCursor(XPM, hotspotX, hotspotY);
  }
  else
  {
    std::cerr << "in mitk::ApplicationCursor::PushCursor(): no implementation registered." << std::endl;
  }
}

void ApplicationCursor::PopCursor()
{
  if (m_Implementation)
  {
    m_Implementation->PopCursor();
  }
  else
  {
    std::cerr << "in mitk::ApplicationCursor::PopCursor(): no implementation registered." << std::endl;
  }
}

} // namespace

