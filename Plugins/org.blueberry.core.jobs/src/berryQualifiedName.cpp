/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
