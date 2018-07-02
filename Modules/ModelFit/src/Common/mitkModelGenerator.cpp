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

#include "mitkModelGenerator.h"
#include "mitkIModelFitProvider.h"

#include "usModuleContext.h"
#include "usGetModuleContext.h"

mitk::IModelFitProvider* mitk::ModelGenerator::GetProviderService(const ModelClassIDType& id)
{
  mitk::IModelFitProvider* result = nullptr;
  std::string filter = "(" + mitk::IModelFitProvider::PROP_MODEL_CLASS_ID() + "=" + id + ")";
  std::vector<us::ServiceReference<mitk::IModelFitProvider> > providerRegisters = us::GetModuleContext()->GetServiceReferences<mitk::IModelFitProvider>(filter);

  if (!providerRegisters.empty())
  {
    if (providerRegisters.size() > 1)
    {
      MITK_WARN << "Multiple provider for class id'"<<id<<"' found. Using just one.";
    }
    result = us::GetModuleContext()->GetService<mitk::IModelFitProvider>(providerRegisters.front());
  }

  return result;
};

mitk::ModelGenerator::ModelGenerator()
= default;

mitk::ModelGenerator::~ModelGenerator()
= default;

mitk::ModelFactoryBase::Pointer mitk::ModelGenerator::GetModelFactory(const ModelClassIDType& id)
{
  mitk::ModelFactoryBase::Pointer factory = nullptr;

  auto service = GetProviderService(id);

  if (service)
  {
    factory = service->GenerateFactory();
  }


  return factory;
}


mitk::ModelParameterizerBase::Pointer mitk::ModelGenerator::GenerateModelParameterizer(
  const modelFit::ModelFitInfo& fit)
{
  mitk::ModelParameterizerBase::Pointer result = nullptr;

  mitk::ModelFactoryBase::Pointer factory = GetModelFactory(fit.functionClassID);

  if (factory.IsNotNull())
  {
    result = factory->CreateParameterizer(&fit);
  }

  return result;
}
