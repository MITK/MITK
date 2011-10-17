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


#include "mitkGroupTagProperty.h"


mitk::GroupTagProperty::GroupTagProperty()
 : mitk::BaseProperty()
{
}


/*!
  Should be implemented by subclasses to indicate whether they can accept the parameter 
  as the right-hand-side argument of an assignment. This test will most probably include
  some dynamic_cast.
 */
bool mitk::GroupTagProperty::Assignable(const BaseProperty& other) const
{
  try
  {
    dynamic_cast<const Self&>(other); // dear compiler, please don't optimize this away!
    return true; 
  }
  catch (std::bad_cast)
  {
  }
  return false;
}

bool mitk::GroupTagProperty::IsEqual(const BaseProperty& property) const
{
  // if other property is also a GroupTagProperty, then it is equal to us, because tags have no value themselves
  return true;
}
