/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCallbackFromGUIThread.h"
#include <mitkLog.h>

mitk::CallbackFromGUIThread *mitk::CallbackFromGUIThread::m_Instance = nullptr;
mitk::CallbackFromGUIThreadImplementation *mitk::CallbackFromGUIThread::m_Implementation = nullptr;

namespace mitk
{
  CallbackFromGUIThread::CallbackFromGUIThread() {}
  CallbackFromGUIThread *CallbackFromGUIThread::GetInstance()
  {
    if (!m_Instance)
    {
      m_Instance = new CallbackFromGUIThread();
    }

    return m_Instance;
  }

  void CallbackFromGUIThread::RegisterImplementation(CallbackFromGUIThreadImplementation *implementation)
  {
    m_Implementation = implementation;
  }

  void CallbackFromGUIThread::CallThisFromGUIThread(itk::Command *cmd, itk::EventObject *e)
  {
    if (m_Implementation)
    {
      m_Implementation->CallThisFromGUIThread(cmd, e);
    }
    else
    {
      MITK_ERROR << "in mitk::CallbackFromGUIThread::CallbackFromGUIThread(): no implementation registered."
                 << std::endl;
    }
  }

} // namespace
