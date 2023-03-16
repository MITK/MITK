/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGenericParamModelFactory_h
#define mitkGenericParamModelFactory_h

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkGenericParamModel.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT GenericParamModelFactory : public
    ConcreteModelFactoryBase<GenericParamModel>
  {
  public:
    mitkClassMacroItkParent(GenericParamModelFactory, ConcreteModelFactoryBase<GenericParamModel>);
    itkFactorylessNewMacro(Self);

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    ParametersType GetDefaultInitialParameterization() const override;

  protected:
    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const override;

    GenericParamModelFactory();

    ~GenericParamModelFactory() override;

  private:

    //No copy constructor allowed
    GenericParamModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}


#endif
