/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPreloadPythonActivator_h
#define mitkPreloadPythonActivator_h

#include <mitkLog.h>
#include <mitkPythonHelper.h>

#include <usModuleActivator.h>

#include <dlfcn.h>

namespace mitk
{
  class PreloadPythonActivator : public us::ModuleActivator
  {
  public:
    PreloadPythonActivator() = default;
    ~PreloadPythonActivator() override = default;

    void Load(us::ModuleContext*) override
    {
      const auto pythonLibrary = PythonHelper::GetLibraryPath();
      MITK_INFO << "Preload Python: " << pythonLibrary.string();
      
      if (dlopen(pythonLibrary.string().c_str(), RTLD_NOW | RTLD_GLOBAL) == nullptr)
        MITK_ERROR << "Failed to preload Python: " << dlerror();
    }

    void Unload(us::ModuleContext*) override
    {
    }
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::PreloadPythonActivator)

#endif

