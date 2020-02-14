/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
