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

#include "berryIStackPresentationSite.h"

namespace berry {

int IStackPresentationSite::STATE_MINIMIZED = 0;
int IStackPresentationSite::STATE_MAXIMIZED = 1;
int IStackPresentationSite::STATE_RESTORED = 2;

IStackPresentationSite::~IStackPresentationSite()
{

}

}
