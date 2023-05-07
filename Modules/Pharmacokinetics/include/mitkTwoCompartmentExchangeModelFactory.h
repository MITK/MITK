/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkTwoCompartmentExchangeModelFactory_h
#define mitkTwoCompartmentExchangeModelFactory_h

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

    ~TwoCompartmentExchangeModelFactory() override;

  private:

    //No copy constructor allowed
    TwoCompartmentExchangeModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };


}
#endif
