/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __EXPONENTIAL_SATURATION_TEST_MODEL_FACTORY_H
#define __EXPONENTIAL_SATURATION_TEST_MODEL_FACTORY_H

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkExponentialSaturationModel.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT ExponentialSaturationModelFactory : public ConcreteModelFactoryBase<ExponentialSaturationModel>
  {
  public:
    mitkClassMacroItkParent(ExponentialSaturationModelFactory, ConcreteModelFactoryBase<ExponentialSaturationModel>);
    itkFactorylessNewMacro(Self);

    ParametersType GetDefaultInitialParameterization() const override;

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override;

  protected:
    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const override;

    ExponentialSaturationModelFactory();

    ~ExponentialSaturationModelFactory() override;

  private:

    //No copy constructor allowed
    ExponentialSaturationModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}


#endif //__EXPONENTIAL_SATURATION_TEST_MODEL_FACTORY_H
