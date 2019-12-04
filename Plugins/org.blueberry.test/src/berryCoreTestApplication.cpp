/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCoreTestApplication.h"

#include <berryPlatform.h>
#include <berryLog.h>

#include "berryBlueBerryTestDriver.h"

namespace berry {

CoreTestApplication::CoreTestApplication()
{

}

CoreTestApplication::CoreTestApplication(const CoreTestApplication& other)
  : QObject()
{
  Q_UNUSED(other)
}

int CoreTestApplication::Start() {
  std::string testPlugin;
  try {
    testPlugin = Platform::GetConfiguration().getString(Platform::ARG_TESTPLUGIN.toStdString());
  }
  catch (const Poco::NotFoundException& /*e*/)
  {
    BERRY_ERROR << "You must specify a test plug-in id via " << Platform::ARG_TESTPLUGIN << "=<id>";
    return 1;
  }

  return BlueBerryTestDriver::Run(QString::fromStdString(testPlugin));
}

void CoreTestApplication::Stop() {

}

}
