/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPreloadPythonActivator_h
#define mitkPreloadPythonActivator_h

#include <mitkEnvironment.h>
#include <mitkLog.h>
#include <mitkPythonHelper.h>

#include <usModuleActivator.h>

#if defined(_WIN32)
#include <array>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace
{
#if defined(_WIN32)
  std::string GetLastErrorAsString()
  {
    std::array<char, 1024> buffer;

    auto size = FormatMessageA(
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      GetLastError(),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      buffer.data(),
      static_cast<DWORD>(buffer.size()),
      nullptr
    );

    return std::string(buffer.data(), size);
  }
#endif
}

namespace mitk
{
  class PreloadPythonActivator : public us::ModuleActivator
  {
  public:
    PreloadPythonActivator() = default;
    ~PreloadPythonActivator() override = default;

    void Load(us::ModuleContext*) override
    {
      const auto pythonHome = PythonHelper::GetHomePath();
      const auto pythonLibrary = PythonHelper::GetLibraryPath();

      MITK_INFO << "Preload Python: " << pythonLibrary.string();

      UnsetEnv("PYTHONHOME");
      UnsetEnv("VIRTUAL_ENV");

#if defined(_WIN32)
      AddPathEnv(pythonHome);
      AddPathEnv(pythonHome / "Scripts");

      // Tell Hugging Face to not use symlinks on Windows as it requires either
      // admin rights or Windows to be in developer mode.
      SetEnv("HF_HUB_DISABLE_SYMLINKS", "1");

      auto handle = LoadLibraryEx(pythonLibrary.string().c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

      if (handle == nullptr)
        MITK_ERROR << "Failed to preload Python: " << GetLastErrorAsString();
#else
      AddPathEnv(pythonHome / "bin");

      if (dlopen(pythonLibrary.string().c_str(), RTLD_NOW | RTLD_GLOBAL) == nullptr)
        MITK_ERROR << "Failed to preload Python: " << dlerror();
#endif
    }

    void Unload(us::ModuleContext*) override
    {
    }
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::PreloadPythonActivator)

#endif

