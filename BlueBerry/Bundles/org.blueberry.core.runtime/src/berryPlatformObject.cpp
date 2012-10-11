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

#include "berryPlatformObject.h"

#include "berryPlatform.h"
#include "berryIAdapterManager.h"

namespace berry {

PlatformObject::PlatformObject()
 : Object()
{

}

Poco::Any PlatformObject::GetAdapter(const std::string& adapter)
{
  IAdapterManager* adapterManager = Platform::GetAdapterManager();
  if (adapterManager)
  {
    return adapterManager->GetAdapter(Object::Pointer(this), adapter);
  }
  else
  {
    return Poco::Any();
  }
}

}
