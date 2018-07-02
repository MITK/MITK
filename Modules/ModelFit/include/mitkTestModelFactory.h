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

#ifndef __TEST_MODEL_FACTORY_H
#define __TEST_MODEL_FACTORY_H

#include <mitkCommon.h>

#include "mitkConcreteModelFactoryBase.h"
#include "mitkTestModel.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT TestModelFactory : public ConcreteModelFactoryBase<TestModel>
  {
  public:
    mitkClassMacro(TestModelFactory, ConcreteModelFactoryBase<TestModel>);
    itkFactorylessNewMacro(Self);

    virtual ParametersType GetDefaultInitialParameterization() const;

  protected:
    virtual ModelParameterizerBase::Pointer DoCreateParameterizer(const modelFit::ModelFitInfo* fit)
    const;

    TestModelFactory();

    virtual ~TestModelFactory();

  private:

    //No copy constructor allowed
    TestModelFactory(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}


#endif //__LINEAR_TEST_MODEL_FACTORY_H
