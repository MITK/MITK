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

#include "mitkPluginActivator.h"

#include <QtPlugin>

#include "QmitkDicomBrowser.h"
#include "QmitkDicomPreferencePage.h"

namespace mitk {
ctkPluginContext* PluginActivator::pluginContext = 0;

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDicomBrowser, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDicomPreferencePage, context)
  pluginContext = context;
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  pluginContext = NULL;
}
ctkPluginContext* PluginActivator::getContext()
{
    return pluginContext;
}
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_dicom, mitk::PluginActivator)
#endif

