/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <berryMacros.h>
#include "berryPluginActivator.h"
#include "berryOSGiCoreTestSuite.h"

namespace berry {

void org_blueberry_osgi_tests_Activator::start(ctkPluginContext* context)
{
  Q_UNUSED(context)

  BERRY_REGISTER_EXTENSION_CLASS(OSGiCoreTestSuite, context)
}

void org_blueberry_osgi_tests_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
