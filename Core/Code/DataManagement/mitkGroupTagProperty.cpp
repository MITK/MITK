/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkGroupTagProperty.h"


mitk::GroupTagProperty::GroupTagProperty()
 : mitk::BaseProperty()
{
}

bool mitk::GroupTagProperty::IsEqual(const BaseProperty& /*property*/) const
{
  // if other property is also a GroupTagProperty, then it is equal to us, because tags have no value themselves
  return true;
}

bool mitk::GroupTagProperty::Assign(const BaseProperty& /*property*/)
{
  return true;
}
