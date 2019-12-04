/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
