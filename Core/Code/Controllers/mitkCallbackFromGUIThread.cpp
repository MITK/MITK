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

#include "mitkCallbackFromGUIThread.h"

mitk::CallbackFromGUIThread* mitk::CallbackFromGUIThread::m_Instance = NULL;
mitk::CallbackFromGUIThreadImplementation* mitk::CallbackFromGUIThread::m_Implementation = NULL;

namespace mitk {

CallbackFromGUIThread::CallbackFromGUIThread()
{
}

CallbackFromGUIThread* CallbackFromGUIThread::GetInstance()
{
  if (!m_Instance)
  {
    m_Instance = new CallbackFromGUIThread();
  }

  return m_Instance;
}
    
void CallbackFromGUIThread::RegisterImplementation(CallbackFromGUIThreadImplementation* implementation)
{
  m_Implementation = implementation;
}

void CallbackFromGUIThread::CallThisFromGUIThread(itk::Command* cmd, itk::EventObject* e)
{
  if (m_Implementation)
  {
    m_Implementation->CallThisFromGUIThread(cmd, e);
  }
  else
  {
    LOG_ERROR << "in mitk::CallbackFromGUIThread::CallbackFromGUIThread(): no implementation registered." << std::endl;
  }
}

} // namespace

