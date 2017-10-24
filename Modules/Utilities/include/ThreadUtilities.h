#pragma once

#include <functional>

#include <boost/logic/tribool.hpp>

#include <MitkUtilitiesExports.h>

namespace Utilities
{
  MITKUTILITIES_EXPORT boost::tribool isGuiThread();

  typedef std::function<void()> ExecuteProc;

  MITKUTILITIES_EXPORT void execInMainThreadAsync(const ExecuteProc& proc);
  MITKUTILITIES_EXPORT void execInMainThreadSync(const ExecuteProc& proc);

  MITKUTILITIES_EXPORT void execUnlocked(const ExecuteProc& proc);
}