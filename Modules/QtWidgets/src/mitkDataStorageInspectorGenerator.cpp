/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usModuleContext.h"
#include "usGetModuleContext.h"

#include "mitkLogMacros.h"
#include "mitkDataStorageInspectorGenerator.h"

mitk::IDataStorageInspectorProvider* mitk::DataStorageInspectorGenerator::GetProvider(const IDType& id)
{
  mitk::IDataStorageInspectorProvider* result = nullptr;

  std::string filter = "(" + mitk::IDataStorageInspectorProvider::PROP_INSPECTOR_ID() + "=" + id + ")";
  std::vector<us::ServiceReference<mitk::IDataStorageInspectorProvider> > providerRegisters = us::GetModuleContext()->GetServiceReferences<mitk::IDataStorageInspectorProvider>(filter);

  if (!providerRegisters.empty())
  {
    if (providerRegisters.size() > 1)
    {
      MITK_WARN << "Multiple provider for class id'" << id << "' found. Using just one.";
    }
    result = us::GetModuleContext()->GetService<mitk::IDataStorageInspectorProvider>(providerRegisters.front());
  }

  return result;
};

mitk::DataStorageInspectorGenerator::ProviderMapType mitk::DataStorageInspectorGenerator::GetProviders()
{
  std::vector<us::ServiceReference<mitk::IDataStorageInspectorProvider> > providerRegisters = us::GetModuleContext()->GetServiceReferences<mitk::IDataStorageInspectorProvider>();

  ProviderMapType result;

  for (const auto &regs : providerRegisters)
  {
    auto provider = us::GetModuleContext()->GetService<mitk::IDataStorageInspectorProvider>(regs);
    result.insert(std::make_pair(provider->GetInspectorID(), provider));
  }

  return result;
};

mitk::DataStorageInspectorGenerator::DataStorageInspectorGenerator()
= default;

mitk::DataStorageInspectorGenerator::~DataStorageInspectorGenerator()
= default;
