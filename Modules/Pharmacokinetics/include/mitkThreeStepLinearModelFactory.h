/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkThreeStepLinearModelFactory_h
#define mitkThreeStepLinearModelFactory_h

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkThreeStepLinearModel.h"
#include "mitkThreeStepLinearModelParameterizer.h"

#include "MitkPharmacokineticsExports.h"
namespace mitk
{


  class MITKPHARMACOKINETICS_EXPORT ThreeStepLinearModelFactory : public ConcreteModelFactoryBase<ThreeStepLinearModel>
  {
  public:
    mitkClassMacroItkParent(ThreeStepLinearModelFactory, ConcreteModelFactoryBase<ThreeStepLinearModel>);
    itkFactorylessNewMacro(Self);

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    ParametersType GetDefaultInitialParameterization() const override;

  protected:
    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const override;

    ThreeStepLinearModelFactory();

    ~ThreeStepLinearModelFactory() override;

  private:

    //No copy constructor allowed
    ThreeStepLinearModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}

#endif
