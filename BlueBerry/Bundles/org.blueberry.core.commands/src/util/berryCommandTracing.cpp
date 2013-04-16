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

const std::string CommandTracing::SEPARATOR = " >>> ";

void CommandTracing::PrintTrace(const std::string& component,
    const std::string& message)
{
  std::string buffer(component);
  if ((!component.empty()) && (!message.empty()))
  {
    buffer += SEPARATOR;
  }
  if (!message.empty())
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
