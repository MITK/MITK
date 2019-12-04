/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkPythonView.h"

namespace mitk
{
    QString PluginActivator::m_XmlFilePath;

    void PluginActivator::start(ctkPluginContext* context)
    {
      m_XmlFilePath = context->getDataFile("PythonSnippets.xml").absoluteFilePath();

      BERRY_REGISTER_EXTENSION_CLASS(QmitkPythonView, context)
    }

    void PluginActivator::stop(ctkPluginContext* context)
    {
        Q_UNUSED(context)
    }
}
