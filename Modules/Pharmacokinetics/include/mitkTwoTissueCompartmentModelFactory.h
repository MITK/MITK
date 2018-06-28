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

#ifndef MITKTWOTISSUECOMPARTMENTMODELFACTORY_H
#define MITKTWOTISSUECOMPARTMENTMODELFACTORY_H

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

    virtual ~TwoTissueCompartmentModelFactory();

  private:

    //No copy constructor allowed
    TwoTissueCompartmentModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented

  };

}

#endif // MITKTWOTISSUECOMPARTMENTMODELFACTORY_H
