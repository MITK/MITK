/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkVirtualEnvUse.h>

#include <mitkPythonHelper.h>
#include <mitkPythonWorker.h>

mitk::VirtualEnvUse mitk::GetVirtualEnvUse(const std::string& venvName)
{
  if (PythonHelper::IsAnyVirtualEnvModuleLoaded(venvName))
    return VirtualEnvUse::LoadedModules;

  const auto python = PythonHelper::GetVirtualEnvExecutablePath(venvName);

  return !python.empty() && PythonWorker::IsAnyRunning(python)
    ? VirtualEnvUse::Worker
    : VirtualEnvUse::None;
}
