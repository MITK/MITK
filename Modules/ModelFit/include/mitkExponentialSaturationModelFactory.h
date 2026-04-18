/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExponentialSaturationModelFactory_h
#define mitkExponentialSaturationModelFactory_h

#include <mitkCommon.h>

#include <mitkConcreteModelFactoryBase.h>
#include <mitkExponentialSaturationModel.h>

#include <MitkModelFitExports.h>

namespace mitk
{

  /**
   * \class ExponentialSaturationModelFactory
   * \brief Factory for creating ExponentialSaturationModel instances and their parameterizers.
   *
   * Provides default initial parameterization and default constraints (e.g. rate > 0).
   *
   * \sa ExponentialSaturationModel, ExponentialSaturationModelParameterizer, ConcreteModelFactoryBase
   */
  class MITKMODELFIT_EXPORT ExponentialSaturationModelFactory : public ConcreteModelFactoryBase<ExponentialSaturationModel>
  {
  public:
    mitkClassMacro(ExponentialSaturationModelFactory, ConcreteModelFactoryBase<ExponentialSaturationModel>);
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


#endif //mitkExponentialSaturationModelFactory_h
