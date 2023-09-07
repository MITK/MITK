/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExponentialDecayModelFactory_h
#define mitkExponentialDecayModelFactory_h

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkExponentialDecayModel.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT ExponentialDecayModelFactory : public ConcreteModelFactoryBase<ExponentialDecayModel>
  {
  public:
    mitkClassMacroItkParent(ExponentialDecayModelFactory, ConcreteModelFactoryBase<ExponentialDecayModel>);
    itkFactorylessNewMacro(Self);

    ParametersType GetDefaultInitialParameterization() const override;

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override;

  protected:
    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const override;

    ExponentialDecayModelFactory();

    ~ExponentialDecayModelFactory() override;

  private:

    //No copy constructor allowed
    ExponentialDecayModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}


#endif
