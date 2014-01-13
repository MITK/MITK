/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY
{
}
 without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkDICOMTag.h"

#include <gdcmGlobal.h>
#include <gdcmDicts.h>

#include "mitkLogMacros.h"

mitk::DICOMTag
::DICOMTag(unsigned int group, unsigned int element)
:m_Group(group)
,m_Element(element)
{
}

mitk::DICOMTag
::DICOMTag(const DICOMTag& other)
:m_Group(other.m_Group)
,m_Element(other.m_Element)
{
}

bool
mitk::DICOMTag
::operator==(const DICOMTag& other) const
{
  return
    m_Group == other.m_Group &&
    m_Element == other.m_Element
    ;
}


mitk::DICOMTag&
mitk::DICOMTag
::operator=(const DICOMTag& other)
{
 if (this != &other)
  {
    m_Group = other.m_Group;
    m_Element = other.m_Element;
  }
  return *this;
}

unsigned int
mitk::DICOMTag
::GetGroup() const
{
  return m_Group;
}

unsigned int
mitk::DICOMTag
::GetElement() const
{
  return m_Element;
}

bool
mitk::DICOMTag
::operator<(const DICOMTag& other) const
{
  return
    ( this->m_Group < other.m_Group )
    ||
    ( ( this->m_Group == other.m_Group )
      &&
      ( this->m_Element < other.m_Element )
    );
}

std::string
mitk::DICOMTag
::GetName() const
{
  gdcm::Tag t(m_Group, m_Element);

  const gdcm::Global& g = gdcm::Global::GetInstance(); // sum of all knowledge !
  const gdcm::Dicts& dicts = g.GetDicts();
  const gdcm::Dict& pub = dicts.GetPublicDict(); // Part 6

  const gdcm::DictEntry& entry = pub.GetDictEntry(t);
  std::string name = entry.GetName();
  if (name.empty())
  {
    name = "Unknown Tag";
  }

  return name;
}

std::string
mitk::DICOMTag
::toHexString(unsigned int i) const
{
  std::stringstream ss;
  ss << std::setfill ('0') << std::setw(4) << std::hex << i;
  return ss.str();
}

void
mitk::DICOMTag
::Print(std::ostream& os) const
{
  os << "(" << toHexString(m_Group) << "," << toHexString(m_Element) << ") " << this->GetName();
}

void
mitk::DICOMStringToOrientationVectors(const std::string& s, Vector3D& right, Vector3D& up, bool& successful)
{
  successful = true;

  std::istringstream orientationReader(s);
  std::string coordinate;
  unsigned int dim(0);
  while( std::getline( orientationReader, coordinate, '\\' ) && dim < 6 )
  {
    if (dim<3)
    {
      right[dim++] = atof(coordinate.c_str());
    }
    else
    {
      up[dim++ - 3] = atof(coordinate.c_str());
    }
  }

  if (dim && dim != 6)
  {
    successful = false;
  }
  else if (dim == 0)
  {
    // fill with defaults
    right.Fill(0.0);
    right[0] = 1.0;

    up.Fill(0.0);
    up[1] = 1.0;

    successful = false;
  }
}

bool
mitk::DICOMStringToSpacing(const std::string& s, ScalarType& spacingX, ScalarType& spacingY)
{
  bool successful = false;

  std::istringstream spacingReader(s);
  std::string spacing;
  if ( std::getline( spacingReader, spacing, '\\' ) )
  {
    spacingY = atof( spacing.c_str() );

    if ( std::getline( spacingReader, spacing, '\\' ) )
    {
      spacingX = atof( spacing.c_str() );

      successful = true;
    }
  }

  return successful;
}

mitk::Point3D
mitk::DICOMStringToPoint3D(const std::string& s, bool& successful)
{
  Point3D p;
  successful = true;

  std::istringstream originReader(s);
  std::string coordinate;
  unsigned int dim(0);
  while( std::getline( originReader, coordinate, '\\' ) && dim < 3)
  {
    p[dim++]= atof(coordinate.c_str());
  }

  if (dim && dim != 3)
  {
    successful = false;
  }
  else if (dim == 0)
  {
    successful = false;
    p.Fill(0.0); // assume default (0,0,0)
  }

  return p;
}
