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

#include "berryPresentationSerializer.h"

#include <Poco/NumberParser.h>
#include <algorithm>
#include <sstream>

namespace berry
{

PresentationSerializer::PresentationSerializer(
    const std::vector<IPresentablePart::Pointer>& presentableParts) :
  parts(presentableParts)
{

}

std::string PresentationSerializer::GetId(IPresentablePart::Pointer part)
{
  std::size_t index = std::find(parts.begin(), parts.end(), part) - parts.begin();

  std::stringstream ssId;
  ssId << index;
  return ssId.str();
}

IPresentablePart::Pointer PresentationSerializer::GetPart(const std::string& id)
{
  try
  {
    unsigned int index = Poco::NumberParser::parseUnsigned(id);

    IPresentablePart::Pointer result;
    if (index < parts.size())
      result = parts[index];

    return result;

  } catch (Poco::SyntaxException& /*e*/)
  {
  }

  return IPresentablePart::Pointer(0);
}

}
