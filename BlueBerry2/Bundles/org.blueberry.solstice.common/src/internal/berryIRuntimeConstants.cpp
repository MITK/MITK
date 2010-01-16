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

#include "berryIRuntimeConstants.h"

namespace berry {

const std::string& IRuntimeConstants::PI_RUNTIME()
{
  static std::string pi_runtime = "org.blueberry.core.runtime";
  return pi_runtime;
}

const std::string& IRuntimeConstants::PI_COMMON()
{
  static std::string pi_common = "org.blueberry.solstice.common";
  return pi_common;
}

const int IRuntimeConstants::PLUGIN_ERROR = 2;
const int IRuntimeConstants::FAILED_WRITE_METADATA = 5;

}
