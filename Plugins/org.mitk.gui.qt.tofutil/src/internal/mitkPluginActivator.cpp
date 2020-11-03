/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
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
