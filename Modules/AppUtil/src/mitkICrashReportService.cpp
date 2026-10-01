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

mitk::ICrashReportService* mitk::GetCrashReportService()
{
  auto* context = us::GetModuleContext();
  if (context == nullptr)
    return nullptr;

  const auto reference = context->GetServiceReference<ICrashReportService>();
  if (!reference)
    return nullptr;

  return context->GetService(reference);
}
