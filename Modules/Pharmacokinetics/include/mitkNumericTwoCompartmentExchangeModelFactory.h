/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKNUMERICTWOCOMPARTMENTEXCHANGEMODELFACTORY_H
#define MITKNUMERICTWOCOMPARTMENTEXCHANGEMODELFACTORY_H

#include "mitkTwoCompartmentExchangeModelFactoryBase.h"
#include "mitkNumericTwoCompartmentExchangeModelParameterizer.h"

namespace mitk
{

  class MITKPHARMACOKINETICS_EXPORT NumericTwoCompartmentExchangeModelFactory : public
    mitk::TwoCompartmentExchangeModelFactoryBase<NumericTwoCompartmentExchangeModelParameterizer>
  {
  public:
    mitkClassMacro(NumericTwoCompartmentExchangeModelFactory,
                   TwoCompartmentExchangeModelFactoryBase<NumericTwoCompartmentExchangeModelParameterizer>);
    itkFactorylessNewMacro(Self);

  protected:

    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit) const override;

    NumericTwoCompartmentExchangeModelFactory();

    ~NumericTwoCompartmentExchangeModelFactory() override;

  private:

    //No copy constructor allowed
    NumericTwoCompartmentExchangeModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif // MITKTWOCOMPARTMENTEXCHANGEMODELFACTORY_H
