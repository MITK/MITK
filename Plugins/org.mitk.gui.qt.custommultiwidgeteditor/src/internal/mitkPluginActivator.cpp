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

#include <QmitkCustomMultiWidgetEditor.h>
#include "QmitkCustomMultiWidgetEditorPreferencePage.h"

namespace mitk
{
  ctkPluginContext* CustomMultiWidgetActivator::m_Context = nullptr;

  void CustomMultiWidgetActivator::start(ctkPluginContext* context)
  {
    m_Context = context;

    BERRY_REGISTER_EXTENSION_CLASS(QmitkCustomMultiWidgetEditor, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkCustomMultiWidgetEditorPreferencePage, context)
  }

  void CustomMultiWidgetActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)

    m_Context = nullptr;
  }

  ctkPluginContext* CustomMultiWidgetActivator::GetContext()
  {
    return m_Context;
  }
}