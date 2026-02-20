/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkRestApiPreferencePage.h"
#include "QmitkRestApiView.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk
{
  ctkPluginContext* RestApiPluginActivator::m_context = nullptr;
  RestApiPluginActivator* RestApiPluginActivator::m_Instance = nullptr;

  RestApiPluginActivator::RestApiPluginActivator()
  {
    m_Instance = this;
  }

  RestApiPluginActivator::~RestApiPluginActivator()
  {
    m_Instance = nullptr;
  }

  void RestApiPluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkRestApiPreferencePage, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkRestApiView, context)
    m_Context = context;
  }

  void RestApiPluginActivator::stop(ctkPluginContext*)
  {
    this->m_context = nullptr;
  }

  RestApiPluginActivator* RestApiPluginActivator::getDefault()
  {
    return m_Instance;
  }

  ctkPluginContext* RestApiPluginActivator::getContext()
  {
    return m_context;
  }
}
