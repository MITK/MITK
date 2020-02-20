/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryDisplay.h"

#include <Poco/Bugcheck.h>

namespace berry {

Display* Display::instance = nullptr;

Display::~Display()
{

}

Display* Display::GetDefault()
{
  poco_assert(instance);
  return instance;
}

}
