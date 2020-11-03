/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryISizeProvider.h"

#include <limits>

namespace berry {

const int ISizeProvider::INF = std::numeric_limits<int>::max();


ISizeProvider::~ISizeProvider()
{
}

}
