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

#include "mitkInteractionDebugger.h"
#include <itkMacro.h>

const char* mitk::InteractionDebugger::m_CurrentSender = "";
bool mitk::InteractionDebugger::m_Active = false;

mitk::InteractionDebugger::InteractionDebugger()
{}

void mitk::InteractionDebugger::Activate()
{
  m_Active = true;
}

void mitk::InteractionDebugger::Deactivate()
{
  m_Active = false;
}
mitk::InteractionDebugger::~InteractionDebugger()
{}

void mitk::InteractionDebugger::Set(const char* sender, const char* text)
{
  if (m_Active)
  {
    itk::OStringStream itkmsg;
    if (! itk::Object::GetGlobalWarningDisplay())
      return;

    if (sender != m_CurrentSender)
    {
      itkmsg << sender <<" :\n"<<text<<"\n";
      m_CurrentSender = sender;
    }
    else
      itkmsg<<text<<"\n";

    itk::OutputWindowDisplayDebugText(itkmsg.str().c_str());
  }
}

