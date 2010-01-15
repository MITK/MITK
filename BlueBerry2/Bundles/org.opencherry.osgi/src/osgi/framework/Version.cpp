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

#include "Version.h"

#include <sstream>

#include <Poco/NumberParser.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Hash.h>
#include <Poco/String.h>

namespace osgi
{
namespace framework
{

const std::string Version::SEPARATOR = ".";

void Version::Validate() const throw (IllegalArgumentException)
{
  if (major < 0)
  {
    throw IllegalArgumentException("negative major");
  }
  if (minor < 0)
  {
    throw IllegalArgumentException("negative minor");
  }
  if (micro < 0)
  {
    throw IllegalArgumentException("negative micro");
  }

  for (std::string::size_type i = 0, length = qualifier.size(); i < length; i++)
  {
    char ch = qualifier[i];
    if (('A' <= ch) && (ch <= 'Z'))
    {
      continue;
    }
    if (('a' <= ch) && (ch <= 'z'))
    {
      continue;
    }
    if (('0' <= ch) && (ch <= '9'))
    {
      continue;
    }
    if ((ch == '_') || (ch == '-'))
    {
      continue;
    }
    throw IllegalArgumentException("invalid qualifier: " + qualifier);
  }
}

const Version Version::emptyVersion = Version(0, 0, 0);

Version::Version(int major, int minor, int micro)
    throw (IllegalArgumentException) :
  major(major), minor(minor), micro(micro), qualifier("")
{

}

Version::Version(int major, int minor, int micro, const std::string& qualifier)
    throw (IllegalArgumentException) :
  major(major), minor(minor), micro(micro), qualifier(qualifier)
{
  this->Validate();
}

Version::Version(const std::string& version) throw (IllegalArgumentException) :
  major(0), minor(0), micro(0), qualifier("")
{
  int maj = 0;
  int min = 0;
  int mic = 0;
  std::string qual = "";

  try
  {
    Poco::StringTokenizer st(version, SEPARATOR);
    Poco::StringTokenizer::Iterator i = st.begin();

    if (i == st.end())
      throw IllegalArgumentException("invalid format");
    maj = Poco::NumberParser::parse(*i);

    if (++i != st.end())
    {
      min = Poco::NumberParser::parse(*i);

      if (++i != st.end())
      {
        mic = Poco::NumberParser::parse(*i);

        if (++i != st.end())
        {
          qual = *i;

          if (++i != st.end())
          {
            throw IllegalArgumentException("invalid format"); //$NON-NLS-1$
          }
        }
      }
    }

  } catch (const Poco::SyntaxException& e)
  {
    throw new IllegalArgumentException("invalid format"); //$NON-NLS-1$
  }

  major = maj;
  minor = min;
  micro = mic;
  qualifier = qual;
  this->Validate();
}

Version::Version(const Version& version) throw()
: major(version.major), minor(version.minor), micro(version.micro), qualifier(
      version.qualifier)
{

}

Version Version::ParseVersion(const std::string& version)
    throw (IllegalArgumentException)
{
  if (version.empty())
  {
    return emptyVersion;
  }

  std::string version2 = Poco::trim(version);
  if (version2.empty())
  {
    return emptyVersion;
  }

  return Version(version2);
}

int Version::GetMajor() const
{
  return major;
}

int Version::GetMinor() const
{
  return minor;
}

int Version::GetMicro() const
{
  return micro;
}

std::string Version::GetQualifier() const
{
  return qualifier;
}

std::string Version::ToString() const
{
  std::stringstream result;
  result << major << SEPARATOR << minor << SEPARATOR << micro;
  if (!qualifier.empty())
  {
    result << SEPARATOR << qualifier;
  }
  return result.str();
}

std::size_t Version::HashCode() const
{
  return (major << 24) + (minor << 16) + (micro << 8) + Poco::hash(qualifier);
}

bool Version::operator==(const Version& other) const
{
  if (&other == this)
  { // quicktest
    return true;
  }

  return (major == other.major) && (minor == other.minor) && (micro
      == other.micro) && qualifier == other.qualifier;
}

bool Version::operator<(const Version& other) const
{
  if (&other == this)
  { // quicktest
    return false;
  }

  if (major < other.major)
  {
    return true;
  }

  if (major == other.major)
  {

    if (minor < other.minor)
    {
      return true;
    }

    if (minor == other.minor)
    {

      if (micro < other.micro)
      {
        return true;
      }

      if (micro == other.micro)
      {
        return qualifier < other.qualifier;
      }
    }
  }
  return false;
}

}
}
