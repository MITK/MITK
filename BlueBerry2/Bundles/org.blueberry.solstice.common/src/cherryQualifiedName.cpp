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

#include "cherryQualifiedName.h"

#include <Poco/Bugcheck.h>
#include <string>
#include <sstream>
#include <Poco/Hash.h>

namespace cherry
{

QualifiedName::QualifiedName(const std::string& qualifier,
    const std::string& localName) :
  qualifier(qualifier), localName(localName)
{
  poco_assert(!localName.empty())
;}

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
::GetLocalName() const
{
  return localName;
}

/**
 * Returns the qualifier part for this qualified name, or <code>null</code>
 * if none.
 *
 * @return the qualifier string, or <code>null</code>
 */
std::string
QualifiedName
::GetQualifier() const
{
  return qualifier;
}

/* (Intentionally omitted from javadoc)
 * Implements the method <code>Object.hashCode</code>.
 *
 * Returns the hash code for this qualified name.
 */
std::size_t
QualifiedName
::HashCode() const
{
  return (qualifier.empty() ? 0 : (Poco::Hash<std::string >().operator()(qualifier) + Poco::Hash<std::string>().operator()(localName)));
}
/**
 * Converts this qualified name into a string, suitable for
 * debug purposes only.
 */
std::string
QualifiedName
::ToString() const
{
  std::string temp = GetQualifier().empty() ? "" : GetQualifier() + ":";
  temp += GetLocalName();
  return  temp;
}

}
