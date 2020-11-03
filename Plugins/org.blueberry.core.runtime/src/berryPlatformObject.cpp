/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPlatformObject.h"

#include "berryPlatform.h"
#include "berryIAdapterManager.h"

namespace berry {

PlatformObject::PlatformObject()
{
}

Object* PlatformObject::GetAdapter(const QString& adapter) const
{
  IAdapterManager* adapterManager = Platform::GetAdapterManager();
  if (adapterManager)
  {
    return adapterManager->GetAdapter(this, adapter);
  }
  else
  {
    return nullptr;
  }
}

}
