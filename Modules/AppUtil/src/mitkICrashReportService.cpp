/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkICrashReportService.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>

mitk::ICrashReportService::~ICrashReportService() = default;

bool mitk::IsCrashReportServiceAvailable()
{
  auto* context = us::GetModuleContext();
  return context != nullptr && static_cast<bool>(context->GetServiceReference<ICrashReportService>());
}

bool mitk::FileCrashReport(const std::vector<CrashDumpInfo>& dumps, QWidget* parent)
{
  auto* context = us::GetModuleContext();
  if (dumps.empty() || context == nullptr)
    return false;

  const auto reference = context->GetServiceReference<ICrashReportService>();
  if (!reference)
    return false;

  auto* service = context->GetService(reference);
  if (service == nullptr)
    return false;

  try
  {
    service->FileReport(dumps, parent);
  }
  catch (...)
  {
    context->UngetService(reference);
    throw;
  }

  context->UngetService(reference);
  return true;
}
