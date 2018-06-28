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

#ifndef __DESCRIPTIVEPHARMACOKINETICBRIXMODEL_FACTORY_H
#define __DESCRIPTIVEPHARMACOKINETICBRIXMODEL_FACTORY_H

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

    virtual ParametersType GetDefaultInitialParameterization() const;

  protected:
    virtual ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const;

    DescriptivePharmacokineticBrixModelFactory();

    virtual ~DescriptivePharmacokineticBrixModelFactory();

  private:

    //No copy constructor allowed
    DescriptivePharmacokineticBrixModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}


#endif //__DESCRIPTIVEPHARMACOKINETICBRIXMODEL_FACTORY_H
