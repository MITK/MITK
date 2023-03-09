/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDescriptivePharmacokineticBrixModelFactory_h
#define mitkDescriptivePharmacokineticBrixModelFactory_h

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkDescriptivePharmacokineticBrixModel.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk
{

  class MITKPHARMACOKINETICS_EXPORT DescriptivePharmacokineticBrixModelFactory : public
    ConcreteModelFactoryBase<DescriptivePharmacokineticBrixModel>
  {
  public:
    mitkClassMacro(DescriptivePharmacokineticBrixModelFactory,
                   ConcreteModelFactoryBase<DescriptivePharmacokineticBrixModel>);
    itkFactorylessNewMacro(Self);

    ParametersType GetDefaultInitialParameterization() const override;

  protected:
    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const override;

    DescriptivePharmacokineticBrixModelFactory();

    ~DescriptivePharmacokineticBrixModelFactory() override;

  private:

    //No copy constructor allowed
    DescriptivePharmacokineticBrixModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}


#endif
