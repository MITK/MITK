/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCommandUtils.h"

namespace berry
{

int CommandUtils::Compare(const bool left, const bool right)
{
  return left == false ? (right == true ? -1 : 0) : (right == true ? 0 : 1);
}

int CommandUtils::Compare(const QString& left, const QString& right)
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
