/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "src/internal/QmitkTubeGraphView.h"

namespace mitk
{

  ctkPluginContext* PluginActivator::m_context = nullptr;

  void PluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkTubeGraphView, context)
    this->m_context = context;
  }

  void PluginActivator::stop(ctkPluginContext*)
  {
    this->m_context = nullptr;
  }

  ctkPluginContext* PluginActivator::getContext()
  {
    return m_context;
  }

} //namespace
