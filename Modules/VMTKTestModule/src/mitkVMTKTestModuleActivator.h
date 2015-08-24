/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkVMTKTestModuleActivator_h
#define mitkVMTKTestModuleActivator_h

#include <usModuleActivator.h>

namespace mitk
{
  class VMTKTestModuleActivator : public us::ModuleActivator
  {
  public:
    VMTKTestModuleActivator();
    ~VMTKTestModuleActivator();

    void Load(us::ModuleContext* context) override;
    void Unload(us::ModuleContext* context) override;

  private:
    VMTKTestModuleActivator(const VMTKTestModuleActivator&);
    VMTKTestModuleActivator& operator=(const VMTKTestModuleActivator&);
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::VMTKTestModuleActivator)

#endif
