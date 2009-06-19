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

#include "cherryPresentationSerializer.h"

#include <Poco/NumberParser.h>
#include <algorithm>

namespace cherry
{

PresentationSerializer::PresentationSerializer(
    const std::vector<IPresentablePart::Pointer>& presentableParts) :
  parts(presentableParts)
{

}

std::string PresentationSerializer::GetId(IPresentablePart::Pointer part)
{
  int index = std::find(parts.begin(), parts.end(), part) - parts.begin();

  return "" + index; //$NON-NLS-1$
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
