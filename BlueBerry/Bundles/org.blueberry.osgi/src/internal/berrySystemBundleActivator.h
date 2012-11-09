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

#ifndef BERRYSYSTEMBUNDLEACTIVATOR_H_
#define BERRYSYSTEMBUNDLEACTIVATOR_H_

#include "berryIBundleActivator.h"

namespace berry {

class SystemBundleActivator : public IBundleActivator
{
public:
  void Start(SmartPointer<IBundleContext> context);
  void Stop(SmartPointer<IBundleContext> context);

};

}

#endif /*BERRYSYSTEMBUNDLEACTIVATOR_H_*/
