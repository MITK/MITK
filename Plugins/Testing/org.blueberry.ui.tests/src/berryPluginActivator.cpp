/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "berryPluginActivator.h"

#include "berryUiTestSuite.h"

#include "api/berryMockViewPart.h"
#include "api/berrySaveableMockViewPart.h"

#include <QtPlugin>

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

Q_EXPORT_PLUGIN2(org_blueberry_ui_tests, berry::org_blueberry_ui_tests_Activator)
