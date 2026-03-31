/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <usModuleActivator.h>

namespace us {

class TestModuleMActivator : public ModuleActivator
{
public:

  void Load(ModuleContext*) override
  {
  }

  void Unload(ModuleContext*) override
  {
  }

};

}

US_EXPORT_MODULE_ACTIVATOR(us::TestModuleMActivator)
