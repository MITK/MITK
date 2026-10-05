/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPluginActivator.h"
#include "QmitkRemeshingView.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

void QmitkPluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkRemeshingView, context);
}

void QmitkPluginActivator::stop(ctkPluginContext*)
{
}
