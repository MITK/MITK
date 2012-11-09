/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryCommandUtils.h"

namespace berry
{

int CommandUtils::Compare(const bool left, const bool right)
{
  return left == false ? (right == true ? -1 : 0) : (right == true ? 0 : 1);
}

int CommandUtils::Compare(const std::string& left, const std::string& right)
{
  return left.compare(right);
}

int CommandUtils::CompareObj(const Object::ConstPointer left,
    const Object::ConstPointer right)
{
  if (!left && !right)
  {
    return 0;
  }
  else if (!left)
  {
    return -1;
  }
  else if (!right)
  {
    return 1;
  }
  else
  {
    return left->ToString().compare(right->ToString());
  }
}

}
