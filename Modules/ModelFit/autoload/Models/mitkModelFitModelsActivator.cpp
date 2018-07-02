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


#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkModelFitProviderBase.h>

//general models
#include "mitkGenericParamModelFactory.h"
#include "mitkLinearModelFactory.h"
#include "mitkT2DecayModelFactory.h"

namespace mitk
{

/*
 * This is the module activator for the IO aspects of the "ModelFit" module.
 */
class ModelFitModelsActivator : public us::ModuleActivator
{
public:

  template <class TProvider> void RegisterProvider(us::ModuleContext* context)
  {
    auto provider = new TProvider();
    provider->RegisterService(context);

    m_RegisteredProviders.push_back(std::unique_ptr<IModelFitProvider>(provider));
  }

  virtual void Load(us::ModuleContext* context) override
  {
    m_RegisteredProviders.clear();
    RegisterProvider<mitk::ModelFitProviderBase<mitk::GenericParamModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::LinearModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::T2DecayModelFactory> >(context);
  }

  virtual void Unload(us::ModuleContext* ) override
  {
  }

private:
  std::vector<std::unique_ptr<IModelFitProvider> > m_RegisteredProviders;
};

}

US_EXPORT_MODULE_ACTIVATOR(mitk::ModelFitModelsActivator)
