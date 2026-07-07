/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTwoStepLinearModelFactory_h
#define mitkTwoStepLinearModelFactory_h

#include <mitkCommon.h>

#include <mitkConcreteModelFactoryBase.h>
#include <mitkTwoStepLinearModel.h>
#include <mitkTwoStepLinearModelParameterizer.h>

#include <MitkModelFitExports.h>

namespace mitk
{

  /**
   * \class TwoStepLinearModelFactory
   * \brief Factory for creating TwoStepLinearModel instances and their parameterizers.
   *
   * \sa TwoStepLinearModel, TwoStepLinearModelParameterizer, ConcreteModelFactoryBase
   */
  class MITKMODELFIT_EXPORT TwoStepLinearModelFactory : public ConcreteModelFactoryBase<TwoStepLinearModel>
  {
  public:
    mitkClassMacro(TwoStepLinearModelFactory, ConcreteModelFactoryBase<TwoStepLinearModel>);
    itkFactorylessNewMacro(Self);

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    ParametersType GetDefaultInitialParameterization() const override;

  protected:
    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const override;

    TwoStepLinearModelFactory();

    virtual ~TwoStepLinearModelFactory();

  private:

    //No copy constructor allowed
    TwoStepLinearModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}

#endif
