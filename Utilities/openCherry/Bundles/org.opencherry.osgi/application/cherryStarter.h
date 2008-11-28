/*=========================================================================

Program:   openCherry Platform
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

#ifndef CHERRYSTARTER_H_
#define CHERRYSTARTER_H_

#include "../cherryOSGiDll.h"

#include <string>

namespace cherry {

class CHERRY_OSGI Starter
{

public:

  static const std::string XP_APPLICATIONS;

  static int Run(int& argc, char** argv);

};

}

#endif /*CHERRYSTARTER_H_*/
