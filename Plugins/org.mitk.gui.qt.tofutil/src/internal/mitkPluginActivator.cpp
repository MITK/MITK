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
#include "QmitkToFUtilView.h"
#include "QmitkToFDeviceGeneration.h"
#include "QmitkToFScreenshotMaker.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
    BERRY_REGISTER_EXTENSION_CLASS(QmitkToFUtilView, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkToFDeviceGeneration, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkToFScreenshotMaker, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
    Q_UNUSED(context)
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_tofutil, mitk::PluginActivator)
#endif
