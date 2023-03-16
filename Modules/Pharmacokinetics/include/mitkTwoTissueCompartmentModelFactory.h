/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTwoTissueCompartmentModelFactory_h
#define mitkTwoTissueCompartmentModelFactory_h

#include "mitkTwoTissueCompartmentModelFactoryBase.h"
#include "mitkTwoTissueCompartmentModelParameterizer.h"

namespace mitk
{


  class MITKPHARMACOKINETICS_EXPORT TwoTissueCompartmentModelFactory : public
    mitk::TwoTissueCompartmentModelFactoryBase<TwoTissueCompartmentModelParameterizer>
  {
  public:
    mitkClassMacro(TwoTissueCompartmentModelFactory,
                   TwoTissueCompartmentModelFactoryBase<TwoTissueCompartmentModelParameterizer>);
    itkFactorylessNewMacro(Self);

  protected:

    TwoTissueCompartmentModelFactory();

    ~TwoTissueCompartmentModelFactory() override;

  private:

    //No copy constructor allowed
    TwoTissueCompartmentModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}

#endif
