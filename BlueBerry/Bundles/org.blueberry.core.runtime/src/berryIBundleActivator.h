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

#ifndef BERRYIBUNDLEACTIVATOR_
#define BERRYIBUNDLEACTIVATOR_

#include <org_blueberry_core_runtime_Export.h>

#include "berrySmartPointer.h"

namespace berry {

struct IBundleContext;

struct org_blueberry_core_runtime_EXPORT IBundleActivator
{
  virtual void Start(SmartPointer<IBundleContext> context) = 0;
  virtual void Stop(SmartPointer<IBundleContext> context) = 0;

  virtual ~IBundleActivator() {};
};

}  // namespace berry

#endif /*BERRYIBUNDLEACTIVATOR_*/
