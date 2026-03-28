/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USTESTDRIVERACTIVATOR_H
#define USTESTDRIVERACTIVATOR_H

#include <usModuleActivator.h>

namespace us {

class TestDriverActivator : public ModuleActivator
{
public:

  TestDriverActivator();

  static bool LoadCalled();

  void Load(ModuleContext*) override;

  void Unload(ModuleContext* ) override;

private:

  static TestDriverActivator* m_Instance;
  bool m_LoadCalled;
};

}

#endif // USTESTDRIVERACTIVATOR_H
