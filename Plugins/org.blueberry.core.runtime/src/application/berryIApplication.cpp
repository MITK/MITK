/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIApplication.h"

namespace berry
{

const int IApplication::EXIT_OK = 0;
const int IApplication::EXIT_RESTART = 23;
const int IApplication::EXIT_RELAUNCH = 24;

IApplication::~IApplication()
{
}

}
