/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelFitInfoSignalGenerationFunctor_h
#define mitkModelFitInfoSignalGenerationFunctor_h

#include "mitkModelBasedValueFunctorBase.h"
#include "mitkModelParameterizerBase.h"
#include "mitkModelFitInfo.h"

#include "mitkModelFitParameterValueExtraction.h"

#include <MitkModelFitExports.h>

namespace  mitk
{

  /** Functor class that can be used to generate a model signal for each index.
    This class is similar to ModelDataGenerationFunctor. But instead of using
    the passed input value as model parameters in the compute function, the model
    fit info is used to deduce the parameters based on the index.
    The time grid and the parameterized model are provided by the model
    parameterizer. This this functor will generate a signal for each index position
    and return the signal as output.*/
  class MITKMODELFIT_EXPORT ModelFitInfoSignalGenerationFunctor : public ModelBasedValueFunctorBase
  {
  public:

    typedef ModelFitInfoSignalGenerationFunctor Self;
    typedef ModelBasedValueFunctorBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkFactorylessNewMacro(Self);
    itkTypeMacro(ModelFitInfoSignalGenerationFunctor, ModelBasedValueFunctorBase);

    typedef std::vector<std::string> ParameterNamesType;
    typedef ModelBase::ModelResultType  SignalType;
    typedef itk::Array<ModelBase::ParameterValueType> ModelParametersType;

    itkSetConstObjectMacro(ModelParameterizer, ModelParameterizerBase);
    itkGetConstObjectMacro(ModelParameterizer, ModelParameterizerBase);

    itkSetConstObjectMacro(FitInfo, mitk::modelFit::ModelFitInfo);
    itkGetConstObjectMacro(FitInfo, mitk::modelFit::ModelFitInfo);

    IndexedValueFunctorBase::OutputPixelVectorType Compute(const InputPixelVectorType & value, const IndexType& currentIndex) const override;
    /** Convinient overload because this functor does not need the value */
    virtual IndexedValueFunctorBase::OutputPixelVectorType Compute(const IndexType& currentIndex) const;

    unsigned int GetNumberOfOutputs() const override;

    GridArrayType GetGrid() const override;

  protected:
    /**Method is called by Compute() to specify the parameters used to generate the model signal for the current index.
     The default implementation just extracts the parameters out of the model fit info and maps it into the result vector.
     Reimplement the method to change this behavior.*/
    virtual ModelBase::ParametersType CompileModelParameters(const IndexType& currentIndex, const ModelBase * model) const;

    ModelFitInfoSignalGenerationFunctor();

    ~ModelFitInfoSignalGenerationFunctor() override;

  private:
    ModelParameterizerBase::ConstPointer m_ModelParameterizer;
    modelFit::ModelFitInfo::ConstPointer m_FitInfo;

  };
}

#endif
