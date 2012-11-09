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

#ifndef BERRYIBUNDLECONTEXT_TXX_
#define BERRYIBUNDLECONTEXT_TXX_

#include "berryPlatform.h"
#include "service/berryServiceRegistry.h"

namespace berry
{

template<class S>
SmartPointer<S> IBundleContext::GetService(const std::string& id) const
{
  return Platform::GetServiceRegistry().GetServiceById<S>(id);
}

}

#endif /*BERRYIBUNDLECONTEXT_TXX_*/
