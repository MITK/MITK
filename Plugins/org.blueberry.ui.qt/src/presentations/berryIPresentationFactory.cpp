/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIPresentationFactory.h"

#include "berryConstants.h"

namespace berry
{

IPresentationFactory::~IPresentationFactory()
{

}

int IPresentationFactory::SASHTYPE_NORMAL = 0;
int IPresentationFactory::SASHTYPE_FLOATING = 1 << 1;
int IPresentationFactory::SASHORIENTATION_HORIZONTAL = Constants::HORIZONTAL; // 1<<8
int IPresentationFactory::SASHORIENTATION_VERTICAL = Constants::VERTICAL; // 1<<9

}
