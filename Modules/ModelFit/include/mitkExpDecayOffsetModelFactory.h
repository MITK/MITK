/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExpDecayOffsetModelFactory_h
#define mitkExpDecayOffsetModelFactory_h

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkExpDecayOffsetModel.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT ExpDecayOffsetModelFactory : public ConcreteModelFactoryBase<ExpDecayOffsetModel>
  {
  public:
    mitkClassMacroItkParent(ExpDecayOffsetModelFactory, ConcreteModelFactoryBase<ExpDecayOffsetModel>);
    itkFactorylessNewMacro(Self);

    ParametersType GetDefaultInitialParameterization() const override;

   //ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override;

  protected:
    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const override;

    ExpDecayOffsetModelFactory();

    ~ExpDecayOffsetModelFactory() override;

  private:

    //No copy constructor allowed
    ExpDecayOffsetModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}


#endif
