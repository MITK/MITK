/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryMultiRule.h"

namespace berry
{

// this is just a dummy implementation of a MultiRule.
bool MultiRule::IsConflicting(ISchedulingRule::Pointer  /*sptr_myRule*/) const
{
  return false;
}

bool MultiRule::Contains(ISchedulingRule::Pointer  /*sptr_myrule*/) const
{
  return false;
}

}
