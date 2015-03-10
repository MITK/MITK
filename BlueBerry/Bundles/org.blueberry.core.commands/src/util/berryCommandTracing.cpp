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
