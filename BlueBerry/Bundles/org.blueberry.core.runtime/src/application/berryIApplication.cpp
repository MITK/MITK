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
