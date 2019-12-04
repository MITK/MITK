/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPluginActivator.h"
#include "berryUiTestSuite.h"
#include "api/berryMockViewPart.h"
#include "api/berrySaveableMockViewPart.h"

namespace berry {

void org_blueberry_ui_tests_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(UiTestSuite, context)
  BERRY_REGISTER_EXTENSION_CLASS(MockViewPart, context)
  BERRY_REGISTER_EXTENSION_CLASS(SaveableMockViewPart, context)
}

void org_blueberry_ui_tests_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
