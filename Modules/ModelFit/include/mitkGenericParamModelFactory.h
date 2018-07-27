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

#ifndef __GENERIC_PARAM_MODEL_FACTORY_H
#define __GENERIC_PARAM_MODEL_FACTORY_H

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkGenericParamModel.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT GenericParamModelFactory : public
    ConcreteModelFactoryBase<GenericParamModel>
  {
  public:
    mitkClassMacroItkParent(GenericParamModelFactory, ConcreteModelFactoryBase<GenericParamModel>);
    itkFactorylessNewMacro(Self);

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    virtual ParametersType GetDefaultInitialParameterization() const override;

  protected:
    virtual ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const override;

    GenericParamModelFactory();

    virtual ~GenericParamModelFactory();

  private:

    //No copy constructor allowed
    GenericParamModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}


#endif //__GENERIC_PARAM_MODEL_FACTORY_H
