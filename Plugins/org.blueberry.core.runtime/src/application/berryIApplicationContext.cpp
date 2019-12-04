/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIApplicationContext.h"

namespace berry {

const QString IApplicationContext::APPLICATION_ARGS = "application.args";
const QString IApplicationContext::APPLICATION_ARGS_UNPROCESSED = "application.args.unprocessed";

IApplicationContext::~IApplicationContext()
{
}

}
