/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryIApplicationContext.h"

namespace berry {

const QString IApplicationContext::APPLICATION_ARGS = "application.args";
const QString IApplicationContext::APPLICATION_ARGS_UNPROCESSED = "application.args.unprocessed";

IApplicationContext::~IApplicationContext()
{
}

}
