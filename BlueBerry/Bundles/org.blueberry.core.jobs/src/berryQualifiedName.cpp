/*=========================================================================

 Program:   BlueBerry Platform
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

#include "berryQualifiedName.h"

#include <berryObject.h>

#include <Poco/Bugcheck.h>
#include <Poco/Hash.h>

#include <string>
#include <sstream>

namespace berry
{

QualifiedName::QualifiedName(std::string qualifier, std::string localName)
{
  poco_assert(!localName.empty());
  this->qualifier = qualifier;
  this->localName = localName;
}

bool
QualifiedName
::operator==(const QualifiedName& qName) const
{
  if (this == &qName) return true;

  return qualifier == qName.GetQualifier() && localName == qName.GetLocalName();
}

bool
QualifiedName
::operator<(const QualifiedName& qName) const
{
  return qualifier < qName.GetQualifier() ? true : (qualifier == qName.GetQualifier() ? localName < qName.GetLocalName() : false);
}

std::string
QualifiedName
::GetLocalName()const
{
  return localName;
}

std::string
QualifiedName
::GetQualifier()const
{
  return qualifier;
}

std::size_t
QualifiedName
::HashCode() const
{
  return (qualifier.empty() ? 0 : (Poco::Hash<std::string >().operator()(qualifier) + Poco::Hash<std::string>().operator()(localName)));
}

std::string
QualifiedName
::ToString()
{
  std::string temp = ( GetQualifier().empty() ? "" : GetQualifier() + ':');
  std::stringstream ss;
  ss << temp << GetLocalName();
  return ss.str();
}

}
