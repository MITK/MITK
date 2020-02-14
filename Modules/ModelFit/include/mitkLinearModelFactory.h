/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __LINEAR_TEST_MODEL_FACTORY_H
#define __LINEAR_TEST_MODEL_FACTORY_H

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkLinearModel.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT LinearModelFactory : public ConcreteModelFactoryBase<LinearModel>
  {
  public:
    mitkClassMacroItkParent(LinearModelFactory, ConcreteModelFactoryBase<LinearModel>);
    itkFactorylessNewMacro(Self);

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    ParametersType GetDefaultInitialParameterization() const override;

  protected:
    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const override;

    LinearModelFactory();

    ~LinearModelFactory() override;

  private:

    //No copy constructor allowed
    LinearModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}


#endif //__LINEAR_TEST_MODEL_FACTORY_H
