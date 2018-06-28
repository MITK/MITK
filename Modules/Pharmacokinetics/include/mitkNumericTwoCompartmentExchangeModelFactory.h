/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
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

    ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit) const;

    NumericTwoCompartmentExchangeModelFactory();

    virtual ~NumericTwoCompartmentExchangeModelFactory();

  private:

    //No copy constructor allowed
    NumericTwoCompartmentExchangeModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}
#endif // MITKTWOCOMPARTMENTEXCHANGEMODELFACTORY_H
