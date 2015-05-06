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

#include "berryIRuntimeConstants.h"

namespace berry {

const QString& IRuntimeConstants::PI_RUNTIME()
{
  static QString pi_runtime = "org.blueberry.core.runtime";
  return pi_runtime;
}

const int IRuntimeConstants::PLUGIN_ERROR = 2;
const int IRuntimeConstants::FAILED_WRITE_METADATA = 5;

}
