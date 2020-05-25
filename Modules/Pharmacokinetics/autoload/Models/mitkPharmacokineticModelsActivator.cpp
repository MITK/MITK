/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkModelFitProviderBase.h>

//MR perfusion models
#include "mitkDescriptivePharmacokineticBrixModelFactory.h"
#include "mitkExtendedToftsModelFactory.h"
#include "mitkStandardToftsModelFactory.h"
#include "mitkTwoCompartmentExchangeModelFactory.h"
#include "mitkNumericTwoCompartmentExchangeModelFactory.h"

//PET perfusion models
#include "mitkOneTissueCompartmentModelFactory.h"
#include "mitkExtendedOneTissueCompartmentModelFactory.h"
#include "mitkTwoTissueCompartmentModelFactory.h"
#include "mitkTwoTissueCompartmentFDGModelFactory.h"
#include "mitkNumericTwoTissueCompartmentModelFactory.h"

//general models
#include "mitkTwoStepLinearModelFactory.h"
#include "mitkThreeStepLinearModelFactory.h"

namespace mitk
{

/*
 * This is the module activator for the IO aspects of the "pharmacokinetics" module.
 */
class PharmacokineticModelsActivator : public us::ModuleActivator
{
public:

  template <class TProvider> void RegisterProvider(us::ModuleContext* context)
  {
    auto provider = new TProvider();
    provider->RegisterService(context);

    m_RegisteredProviders.push_back(std::unique_ptr<IModelFitProvider>(provider));
  }

  void Load(us::ModuleContext* context) override
  {
    m_RegisteredProviders.clear();
    RegisterProvider<mitk::ModelFitProviderBase<mitk::DescriptivePharmacokineticBrixModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::ThreeStepLinearModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::TwoStepLinearModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::ExtendedToftsModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::StandardToftsModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::TwoCompartmentExchangeModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::NumericTwoCompartmentExchangeModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::OneTissueCompartmentModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::ExtendedOneTissueCompartmentModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::TwoTissueCompartmentFDGModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::TwoTissueCompartmentModelFactory> >(context);
    RegisterProvider<mitk::ModelFitProviderBase<mitk::NumericTwoTissueCompartmentModelFactory> >(context);
  }

  void Unload(us::ModuleContext* ) override
  {
  }

private:
  std::vector<std::unique_ptr<IModelFitProvider> > m_RegisteredProviders;
};

}

US_EXPORT_MODULE_ACTIVATOR(mitk::PharmacokineticModelsActivator)
