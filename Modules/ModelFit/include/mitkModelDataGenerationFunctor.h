/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKMODELDATAGENERATIONFUNCTOR_H
#define MITKMODELDATAGENERATIONFUNCTOR_H

#include "mitkSimpleFunctorBase.h"
#include "mitkModelBase.h"
#include "mitkModelParameterizerBase.h"

#include <MitkModelFitExports.h>

namespace  mitk
{

  /** Functor class that can be used to generate a model signal for each index.
    This class assumes that value parameter passed in the Compute() call,
    is the parameter vector that should be used to generate the model signal.
    The time grid and the parameterized model are provided by the model
    parameterizer. This this functor will generate a signal for each index position
    and return the signal as output.*/
  class MITKMODELFIT_EXPORT ModelDataGenerationFunctor : public SimpleFunctorBase
  {
  public:

    typedef ModelDataGenerationFunctor Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkFactorylessNewMacro(Self);
    itkTypeMacro(ModelDataGenerationFunctor, SimpleFunctorBase);

    typedef std::vector<std::string> ParameterNamesType;
    typedef ModelBase::ModelResultType  SignalType;
    typedef itk::Array<ModelBase::ParameterValueType> ModelParametersType;

    itkSetConstObjectMacro(ModelParameterizer, ModelParameterizerBase);
    itkGetConstObjectMacro(ModelParameterizer, ModelParameterizerBase);

    SimpleFunctorBase::OutputPixelVectorType Compute(const InputPixelVectorType & value) const override;

    unsigned int GetNumberOfOutputs() const override;

    GridArrayType GetGrid() const override;

  protected:
    ModelDataGenerationFunctor();
    ~ModelDataGenerationFunctor() override;

  private:
    ModelParameterizerBase::ConstPointer m_ModelParameterizer;

  };
}

#endif // MITKMODELDATAGENERATIONFUNCTOR_H
