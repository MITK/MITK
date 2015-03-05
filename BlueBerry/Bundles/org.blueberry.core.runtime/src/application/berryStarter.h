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

#ifndef BERRYSTARTER_H_
#define BERRYSTARTER_H_

#include <org_blueberry_core_runtime_Export.h>

#include "berryPlatform.h"
#include <string>

#include <Poco/Path.h>

namespace berry {

class org_blueberry_core_runtime_EXPORT Starter
{

public:

  static const QString XP_APPLICATIONS;

  static int Run(int& argc, char** argv, Poco::Util::AbstractConfiguration* config = 0);

};

}

#endif /*BERRYSTARTER_H_*/
