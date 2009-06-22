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

/*!
  Should be implemented by subclasses to indicate whether they can accept the parameter 
  as the right-hand-side argument of an assignment. This test will most probably include
  some dynamic_cast.
 */
bool mitk::BaseProperty::Assignable( const BaseProperty& ) const
{
  return false;
}

/*!
  To be implemented more meaningful by subclasses. This version just accepts the assignment
  of BaseProperty objects to others, but the assignment has NO MEANING, values are not changed at all!
 */
mitk::BaseProperty& mitk::BaseProperty::operator=(const BaseProperty& rhs)
{
  if (this == &rhs) return *this; // no self assignment

  // place meaningful copy code here (nothing possible with BaseProeprties)

  return *this;
}
