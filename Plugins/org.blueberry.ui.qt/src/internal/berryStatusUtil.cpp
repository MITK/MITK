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
