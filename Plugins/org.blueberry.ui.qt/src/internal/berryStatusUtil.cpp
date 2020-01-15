/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryStatusUtil.h"

#include "berryPlatformUI.h"

namespace berry {

IStatus::Pointer StatusUtil::NewStatus(IStatus::Severity severity, const QString& message,
                                       const Status::SourceLocation& sl)
{
  IStatus::Pointer status(new Status(severity, PlatformUI::PLUGIN_ID(), severity, message, sl));
  return status;
}

IStatus::Pointer StatusUtil::NewStatus(IStatus::Severity severity, const QString& message,
                                  const ctkException& exc, const Status::SourceLocation& sl)
{
  QString statusMessage = message;
  if (message.isNull() || message.trimmed().isEmpty())
  {
    if (exc.message().isEmpty())
    {
      statusMessage = exc.what();
    }
    else
    {
      statusMessage = exc.message();
    }
  }

  IStatus::Pointer status(new Status(severity, PlatformUI::PLUGIN_ID(), severity,
                                     statusMessage, GetCause(exc), sl));
  return status;
}

ctkException StatusUtil::GetCause(const ctkException& exc)
{
  const ctkException* cause = exc.cause();
  if (cause)
  {
    return *cause;
  }
  return exc;
}

}
