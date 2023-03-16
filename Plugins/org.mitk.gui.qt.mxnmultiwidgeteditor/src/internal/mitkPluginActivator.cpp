/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"

#include <QmitkMxNMultiWidgetEditor.h>
#include "QmitkMxNMultiWidgetEditorPreferencePage.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk
{
  ctkPluginContext* MxNMultiWidgetActivator::m_Context = nullptr;

  void MxNMultiWidgetActivator::start(ctkPluginContext* context)
  {
    m_Context = context;

    BERRY_REGISTER_EXTENSION_CLASS(QmitkMxNMultiWidgetEditor, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkMxNMultiWidgetEditorPreferencePage, context)
  }

  void MxNMultiWidgetActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)

    m_Context = nullptr;
  }

  ctkPluginContext* MxNMultiWidgetActivator::GetContext()
  {
    return m_Context;
  }
}
