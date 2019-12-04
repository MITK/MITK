/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <berryMacros.h>
#include "berryPluginActivator.h"
#include "berryCoreRuntimeTestSuite.h"

namespace berry {

void org_blueberry_core_runtime_tests_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(CoreRuntimeTestSuite, context)
}

void org_blueberry_core_runtime_tests_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
