/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPluginActivator.h"

#include <QmitkMxNMultiWidgetEditor.h>
#include "QmitkMxNMultiWidgetEditorPreferencePage.h"

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