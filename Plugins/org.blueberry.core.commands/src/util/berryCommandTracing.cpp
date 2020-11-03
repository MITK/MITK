/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryLog.h"

#include "berryCommandTracing.h"

#include <iostream>

namespace berry {

const QString CommandTracing::SEPARATOR = " >>> ";

void CommandTracing::PrintTrace(const QString& component,
    const QString& message)
{
  QString buffer(component);
  if ((!component.isEmpty()) && (!message.isEmpty()))
  {
    buffer += SEPARATOR;
  }
  if (!message.isEmpty())
  {
    buffer += message;
  }
  BERRY_INFO << buffer;
}

CommandTracing::CommandTracing()
{
  // Do nothing.
}

}
