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
