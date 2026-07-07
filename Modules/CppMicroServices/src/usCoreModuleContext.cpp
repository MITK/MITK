/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <usGlobalConfig.h>

US_MSVC_DISABLE_WARNING(4355)

#include "usCoreModuleContext_p.h"

namespace us {

CoreModuleContext::CoreModuleContext()
  : listeners(this)
  , services(this)
  , serviceHooks(this)
  , moduleHooks(this)
{
}

CoreModuleContext::~CoreModuleContext()
{
}

void CoreModuleContext::Init()
{
  serviceHooks.Open();
}

void CoreModuleContext::Uninit()
{
  serviceHooks.Close();
}

}
