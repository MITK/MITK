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

#ifndef __T2_DECAY_TEST_MODEL_FACTORY_H
#define __T2_DECAY_TEST_MODEL_FACTORY_H

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkT2DecayModel.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT T2DecayModelFactory : public ConcreteModelFactoryBase<T2DecayModel>
  {
  public:
    mitkClassMacroItkParent(T2DecayModelFactory, ConcreteModelFactoryBase<T2DecayModel>);
    itkFactorylessNewMacro(Self);

    virtual ParametersType GetDefaultInitialParameterization() const override;

    ConstraintCheckerBase::Pointer CreateDefaultConstraints() const override;

  protected:
    virtual ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const;

    T2DecayModelFactory();

    virtual ~T2DecayModelFactory();

  private:

    //No copy constructor allowed
    T2DecayModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}


#endif //__LINEAR_TEST_MODEL_FACTORY_H
