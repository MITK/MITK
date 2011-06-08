/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include <berryMacros.h>

#include "berryPluginActivator.h"

#include "berryCoreRuntimeTestSuite.h"

#include <QtPlugin>

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

Q_EXPORT_PLUGIN2(org_blueberry_core_runtime_tests, berry::org_blueberry_core_runtime_tests_Activator)
