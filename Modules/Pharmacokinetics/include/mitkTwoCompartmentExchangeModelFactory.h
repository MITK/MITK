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
#ifndef MITKTWOCOMPARTMENTEXCHANGEMODELFACTORY_H
#define MITKTWOCOMPARTMENTEXCHANGEMODELFACTORY_H

#include "mitkTwoCompartmentExchangeModelFactoryBase.h"
#include "mitkTwoCompartmentExchangeModelParameterizer.h"

namespace mitk
{
  class MITKPHARMACOKINETICS_EXPORT TwoCompartmentExchangeModelFactory : public
    mitk::TwoCompartmentExchangeModelFactoryBase<TwoCompartmentExchangeModelParameterizer>
  {
  public:
    mitkClassMacro(TwoCompartmentExchangeModelFactory,
                   TwoCompartmentExchangeModelFactoryBase<TwoCompartmentExchangeModelParameterizer>);
    itkFactorylessNewMacro(Self);

  protected:

    TwoCompartmentExchangeModelFactory();

    virtual ~TwoCompartmentExchangeModelFactory();

  private:

    //No copy constructor allowed
    TwoCompartmentExchangeModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };


}
#endif // MITKTWOCOMPARTMENTEXCHANGEMODELFACTORY_H
