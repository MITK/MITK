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

#ifndef __DUMMYMODELFITFUNCTOR_H
#define __DUMMYMODELFITFUNCTOR_H

#include <itkObject.h>

#include "mitkModelBase.h"
#include "mitkModelFitFunctorBase.h"
#include "mitkMVConstrainedCostFunctionDecorator.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT DummyModelFitFunctor : public ModelFitFunctorBase
  {
  public:
    typedef DummyModelFitFunctor Self;
    typedef ModelFitFunctorBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(DummyModelFitFunctor, ModelFitFunctorBase);

    typedef Superclass::InputPixelArrayType InputPixelArrayType;
    typedef Superclass::OutputPixelArrayType OutputPixelArrayType;

    itkSetMacro(DerivativeStepLength, double);
    itkGetMacro(DerivativeStepLength, double);

    virtual ParameterNamesType GetCriterionNames() const;

  protected:

    typedef Superclass::ParametersType ParametersType;
    typedef Superclass::SignalType SignalType;

    DummyModelFitFunctor();

    ~DummyModelFitFunctor();

    virtual ParametersType DoModelFit(const SignalType& value, const ModelBase* model,
                                      const ModelBase::ParametersType& initialParameters,
                                      DebugParameterMapType& debugParameters) const;

    virtual OutputPixelArrayType GetCriteria(const ModelBase* model, const ParametersType& parameters,
        const SignalType& sample) const;

    /** Generator function that instantiates and parameterizes the cost function that should be used by the fit functor*/
    virtual MVModelFitCostFunction::Pointer GenerateCostFunction(const SignalType& value,
        const ModelBase* model) const;

    virtual ParameterNamesType DefineDebugParameterNames() const;

  private:
    double m_DerivativeStepLength;

  };

}


#endif // __DUMMYMODELFITFUNCTOR_H
