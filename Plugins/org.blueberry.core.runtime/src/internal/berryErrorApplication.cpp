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

#include "berryErrorApplication.h"

#include "berryIApplicationContext.h"

#include <ctkException.h>

#include <QVariant>

namespace berry {

const QString ErrorApplication::ERROR_EXCEPTION = "error.exception";

QVariant ErrorApplication::Start(IApplicationContext* context)
{
  QString errorStr = context->GetArguments()[ERROR_EXCEPTION].toString();
  if (!errorStr.isEmpty())
  {
    throw ctkRuntimeException(errorStr);
  }
  throw ctkIllegalStateException("");
}

void ErrorApplication::Stop()
{
  // do nothing
}

}
