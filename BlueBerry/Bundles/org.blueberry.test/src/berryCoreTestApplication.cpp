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

#include "berryCoreTestApplication.h"

#include <berryPlatform.h>
#include <berryLog.h>

#include "berryBlueBerryTestDriver.h"

namespace berry {

CoreTestApplication::CoreTestApplication()
{

}

CoreTestApplication::CoreTestApplication(const CoreTestApplication& other)
{
  Q_UNUSED(other)
}

int CoreTestApplication::Start() {
  std::string testPlugin;
  try {
    testPlugin = Platform::GetConfiguration().getString(Platform::ARG_TESTPLUGIN);
  }
  catch (const Poco::NotFoundException& /*e*/)
  {
    BERRY_ERROR << "You must specify a test plug-in id via " << Platform::ARG_TESTPLUGIN << "=<id>";
    return 1;
  }

  return BlueBerryTestDriver::Run(testPlugin);
}

void CoreTestApplication::Stop() {

}

}
