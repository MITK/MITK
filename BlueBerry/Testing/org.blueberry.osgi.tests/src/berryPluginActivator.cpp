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

#include <berryMacros.h>

#include "berryPluginActivator.h"

#include "berryOSGiCoreTestSuite.h"

#include <QtPlugin>

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

Q_EXPORT_PLUGIN2(org_blueberry_osgi_tests, berry::org_blueberry_osgi_tests_Activator)
