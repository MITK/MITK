/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/


#include <usGlobalConfig.h>

US_MSVC_DISABLE_WARNING(4355)

#include "usCoreModuleContext_p.h"

US_BEGIN_NAMESPACE

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

US_END_NAMESPACE
