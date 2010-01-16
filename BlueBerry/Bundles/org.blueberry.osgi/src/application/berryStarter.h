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

#include "../berryOSGiDll.h"

#include "../berryPlatform.h"
#include <string>

namespace berry {

class BERRY_OSGI Starter
{

public:

  static const std::string XP_APPLICATIONS;

  static int Run(int& argc, char** argv, Poco::Util::AbstractConfiguration* config = 0);

};

}

#endif /*BERRYSTARTER_H_*/
