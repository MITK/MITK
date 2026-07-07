/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usTestDriverActivator.h"
#include <usModuleImport.h>

namespace us {

TestDriverActivator* TestDriverActivator::m_Instance = nullptr;

TestDriverActivator::TestDriverActivator()
  : m_LoadCalled(false)
{
}

bool TestDriverActivator::LoadCalled()
{
  return m_Instance ? m_Instance->m_LoadCalled : false;
}

void TestDriverActivator::Load(ModuleContext*)
{
  this->m_Instance = this;
  this->m_LoadCalled = true;
}

void TestDriverActivator::Unload(ModuleContext*)
{
  this->m_Instance = nullptr;
}

}

US_EXPORT_MODULE_ACTIVATOR(us::TestDriverActivator)
