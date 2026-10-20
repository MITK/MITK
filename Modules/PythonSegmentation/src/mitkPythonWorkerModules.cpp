/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPythonWorkerModules.h"

#include <mitkExceptionMacro.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <filesystem>
#include <sstream>

mitk::PythonWorker::Module mitk::ReadWorkerModule(const std::string& resourcePath)
{
  auto resource = us::GetModuleContext()->GetModule()->GetResource(resourcePath);

  if (!resource.IsValid())
    mitkThrow() << "The resource " << resourcePath << " could not be found.";

  us::ModuleResourceStream stream(resource);

  std::ostringstream source;
  source << stream.rdbuf();

  return { std::filesystem::path(resourcePath).stem().string(), source.str() };
}

nlohmann::json mitk::GetIndexToWorld(const BaseGeometry* geometry)
{
  const auto* transform = geometry->GetIndexToWorldTransform();
  const auto& matrix = transform->GetMatrix();
  const auto& offset = transform->GetOffset();

  auto rows = nlohmann::json::array();

  for (unsigned int row = 0; row < 3; ++row)
    rows.push_back({ matrix[row][0], matrix[row][1], matrix[row][2], offset[row] });

  rows.push_back({ 0.0, 0.0, 0.0, 1.0 });

  return rows;
}

std::exception_ptr mitk::MakeExceptionPtr(const std::string& message)
{
  try
  {
    mitkThrow() << message;
  }
  catch (...)
  {
    return std::current_exception();
  }
}
