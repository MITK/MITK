/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MODEL_GENERATOR_H
#define MODEL_GENERATOR_H

#include "mitkModelBase.h"
#include "mitkModelFactoryBase.h"
#include "mitkModelParameterizerBase.h"

#include "mitkModelFitInfo.h"

#include "mitkIModelFitProvider.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT ModelGenerator
  {
  public:
    typedef ModelFactoryBase::ModellClassIDType ModelClassIDType;

    /** Returns the pointer to a factory instance that is responsible for the
     given model class id.
     If no fitting factory can be found, a null pointer is returned.
    */
    static ModelFactoryBase::Pointer GetModelFactory(const ModelClassIDType& id);

    /** Takes a fit instances and generates the corresponding ModelParameterizer.
     If no fitting factory can be found or no class id is specified in the fit, a null pointer is returned.
    */
    static ModelParameterizerBase::Pointer GenerateModelParameterizer(const modelFit::ModelFitInfo&
        fit);

    static IModelFitProvider* GetProviderService(const ModelClassIDType& id);

  protected:
    ModelGenerator();
    virtual ~ModelGenerator();

  private:
    //No copy constructor allowed
    ModelGenerator(const ModelGenerator& source);
    void operator=(const ModelGenerator&);  //purposely not implemented
  };
}

#endif // MODEL_GENERATOR_H
