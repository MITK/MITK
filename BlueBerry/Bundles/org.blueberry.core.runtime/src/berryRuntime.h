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

#ifndef BERRYRUNTIME_H_
#define BERRYRUNTIME_H_

#include <string>

#include "berryRuntimeDll.h"

namespace berry {

struct BERRY_RUNTIME Runtime
{
  static const std::string ADAPTER_SERVICE_ID;
};

}

#endif /*BERRYRUNTIME_H_*/
