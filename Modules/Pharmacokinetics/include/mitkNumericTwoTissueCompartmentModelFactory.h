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

#ifndef MITKNUMERICTWOTISSUECOMPARTMENTMODELFACTORY_H
#define MITKNUMERICTWOTISSUECOMPARTMENTMODELFACTORY_H

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

    virtual ~NumericTwoTissueCompartmentModelFactory();

  private:

    //No copy constructor allowed
    NumericTwoTissueCompartmentModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}


#endif // MITKNUMERICTWOTISSUECOMPARTMENTMODELFACTORY_H
