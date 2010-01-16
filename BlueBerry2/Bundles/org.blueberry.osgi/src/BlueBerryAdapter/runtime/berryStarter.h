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

#ifndef BERRYSTARTER_H_
#define BERRYSTARTER_H_

#include "../../berryOSGiDll.h"

#include <string>

#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Exception.h>

#include <osgi/framework/Object.h>

namespace berry {

class BERRY_OSGI Starter
{

public:

  static const std::string XP_APPLICATIONS;

  static int Run(int& argc, char** argv, Poco::Util::AbstractConfiguration* config = 0);

  /**
   * Runs the application for which the platform was started. The platform
   * must be running.
   * <p>
   * The given argument is passed to the application being run.  If it is <code>null</code>
   * then the command line arguments used in starting the platform, and not consumed
   * by the platform code, are passed to the application as a <code>String[]</code>.
   * </p>
   * @param argument the argument passed to the application. May be <code>null</code>
   * @return the result of running the application
   * @throws Poco::Exception if anything goes wrong
   */
  static osgi::framework::Object::Pointer Run(osgi::framework::Object::Pointer argument) throw(Poco::Exception);

};

}

#endif /*BERRYSTARTER_H_*/
