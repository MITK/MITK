/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNumericTwoTissueCompartmentModelFactory_h
#define mitkNumericTwoTissueCompartmentModelFactory_h

#include "mitkTwoTissueCompartmentModelFactoryBase.h"
#include "mitkNumericTwoTissueCompartmentModelParameterizer.h"

namespace mitk
{


  class MITKPHARMACOKINETICS_EXPORT NumericTwoTissueCompartmentModelFactory : public
    mitk::TwoTissueCompartmentModelFactoryBase<NumericTwoTissueCompartmentModelParameterizer>
  {
  public:
    mitkClassMacro(NumericTwoTissueCompartmentModelFactory,
                   TwoTissueCompartmentModelFactoryBase<NumericTwoTissueCompartmentModelParameterizer>);
    itkFactorylessNewMacro(Self);

  protected:

    NumericTwoTissueCompartmentModelFactory();

    ~NumericTwoTissueCompartmentModelFactory() override;

  private:

    //No copy constructor allowed
    NumericTwoTissueCompartmentModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}


#endif
