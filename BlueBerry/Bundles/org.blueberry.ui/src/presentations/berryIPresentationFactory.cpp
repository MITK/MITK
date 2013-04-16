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
