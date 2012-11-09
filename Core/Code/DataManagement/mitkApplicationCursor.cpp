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

#include "mitkApplicationCursor.h"
#include <mitkLogMacros.h>

#include <iostream>

mitk::ApplicationCursorImplementation* mitk::ApplicationCursor::m_Implementation = NULL;

namespace mitk {

ApplicationCursor::ApplicationCursor()
{
}

ApplicationCursor* ApplicationCursor::GetInstance()
{
  static ApplicationCursor* m_Instance = NULL;
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
    MITK_ERROR << "in mitk::ApplicationCursor::PushCursor(): no implementation registered." << std::endl;
    throw std::logic_error("No implementation registered for mitk::ApplicationCursor.");
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
    MITK_ERROR << "in mitk::ApplicationCursor::PopCursor(): no implementation registered." << std::endl;
    throw std::logic_error("No implementation registered for mitk::ApplicationCursor.");
  }
}

const Point2I ApplicationCursor::GetCursorPosition()
{
  if (m_Implementation)
  {
    return m_Implementation->GetCursorPosition();
  }
  else
  {
    MITK_ERROR << "in mitk::ApplicationCursor::GetCursorPosition(): no implementation registered." << std::endl;
    throw std::logic_error("No implementation registered for mitk::ApplicationCursor.");
  }
}

void ApplicationCursor::SetCursorPosition(const Point2I& p)
{
  if (m_Implementation)
  {
    m_Implementation->SetCursorPosition(p);
  }
  else
  {
    MITK_ERROR << "in mitk::ApplicationCursor::SetCursorPosition(): no implementation registered." << std::endl;
    throw std::logic_error("No implementation registered for mitk::ApplicationCursor.");
  }
}

} // namespace

