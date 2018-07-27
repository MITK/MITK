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

  for (auto regs : providerRegisters)
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