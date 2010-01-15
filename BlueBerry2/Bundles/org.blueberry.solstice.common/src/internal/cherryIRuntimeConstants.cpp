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

#include "cherryIRuntimeConstants.h"

namespace cherry {

const std::string& IRuntimeConstants::PI_RUNTIME()
{
  static std::string pi_runtime = "org.opencherry.core.runtime";
  return pi_runtime;
}

const std::string& IRuntimeConstants::PI_COMMON()
{
  static std::string pi_common = "org.opencherry.solstice.common";
  return pi_common;
}

const int IRuntimeConstants::PLUGIN_ERROR = 2;
const int IRuntimeConstants::FAILED_WRITE_METADATA = 5;

}
