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

#include "berryCoreTestApplication.h"

#include <berryPlatform.h>
#include <berryLog.h>

#include "berryIBlueBerryTestDriver.h"

namespace berry {

int CoreTestApplication::Start() {
  std::string testPlugin;
  try {
    testPlugin = Platform::GetConfiguration().getString(Platform::ARG_TESTPLUGIN);
  }
  catch (const Poco::NotFoundException& e)
  {
    BERRY_ERROR << "You must specify a test plug-in id via " << Platform::ARG_TESTPLUGIN << "=<id>";
    return 1;
  }

  return BlueBerryTestDriver::Run(testPlugin);
}

void CoreTestApplication::Stop() {

}

}
