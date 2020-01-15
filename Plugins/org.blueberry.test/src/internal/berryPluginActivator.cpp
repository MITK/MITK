/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPluginActivator.h"
#include "berryMacros.h"
#include "berryCoreTestApplication.h"

namespace berry {

org_blueberry_test_Activator::org_blueberry_test_Activator()
{

}

void org_blueberry_test_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(CoreTestApplication, context)
}

void org_blueberry_test_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
