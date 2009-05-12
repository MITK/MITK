/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryCommandTracing.h"

#include <iostream>

namespace cherry {

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
  std::cout << buffer;
}

CommandTracing::CommandTracing()
{
  // Do nothing.
}

}
