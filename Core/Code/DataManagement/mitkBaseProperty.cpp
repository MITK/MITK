/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkBaseProperty.h"


mitk::BaseProperty::BaseProperty() 
{
}

mitk::BaseProperty::~BaseProperty()
{
}

std::string mitk::BaseProperty::GetValueAsString() const
{
  return std::string("n/a"); 
}

mitk::BaseProperty& mitk::BaseProperty::operator=(const BaseProperty& rhs)
{
  AssignProperty(rhs);
  return *this;
}

bool mitk::BaseProperty::AssignProperty(const BaseProperty& rhs)
{
  if (this == &rhs) return true; // no self assignment

  const char* t1 = typeid(this).name();
  const char* t2 = typeid(&rhs).name();

  std::string s1(t1);
  std::string s2(t2);

  if (typeid(*this) == typeid(rhs) && Assign(rhs))
  {
    this->Modified();
    return true;
  }
  return false;
}

bool mitk::BaseProperty::operator==(const BaseProperty& property) const
{
  return (typeid(*this) == typeid(property) && IsEqual(property));
}
