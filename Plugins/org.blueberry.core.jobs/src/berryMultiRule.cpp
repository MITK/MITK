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
