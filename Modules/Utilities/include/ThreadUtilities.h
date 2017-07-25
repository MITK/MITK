#pragma once

#include <functional>

#include <MitkUtilitiesExports.h>

namespace Utilities
{
  MITKUTILITIES_EXPORT bool isGuiThread();

  typedef std::function<void()> ExecuteProc;

  MITKUTILITIES_EXPORT void execInMainThreadAsync(const ExecuteProc& proc);
  MITKUTILITIES_EXPORT void execInMainThreadSync(const ExecuteProc& proc);

  MITKUTILITIES_EXPORT void execUnlocked(const ExecuteProc& proc);
}