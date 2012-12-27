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

#include "berryQtDnDTweaklet.h"

#include "berryQtTracker.h"

namespace berry {

QtDnDTweaklet::QtDnDTweaklet()
{

}

QtDnDTweaklet::QtDnDTweaklet(const QtDnDTweaklet& other)
{
  Q_UNUSED(other)
}

ITracker* QtDnDTweaklet::CreateTracker()
{
  return new QtTracker();
}

}
