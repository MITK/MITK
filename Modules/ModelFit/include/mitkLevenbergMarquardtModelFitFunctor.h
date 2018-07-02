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

#ifndef LEVENBERGMARQUARDTMODELFITFUNCTOR_H
#define LEVENBERGMARQUARDTMODELFITFUNCTOR_H

#include <itkObject.h>
#include <itkLevenbergMarquardtOptimizer.h>

#include "mitkModelBase.h"
#include "mitkModelFitFunctorBase.h"
#include "mitkMVConstrainedCostFunctionDecorator.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT LevenbergMarquardtModelFitFunctor : public ModelFitFunctorBase
  {
  public:
    typedef LevenbergMarquardtModelFitFunctor Self;
    typedef ModelFitFunctorBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(LevenbergMarquardtModelFitFunctor, ModelFitFunctorBase);

    typedef Superclass::InputPixelArrayType InputPixelArrayType;
    typedef Superclass::OutputPixelArrayType OutputPixelArrayType;

    itkSetMacro(Epsilon, double);
    itkSetMacro(GradientTolerance, double);
    itkSetMacro(ValueTolerance, double);
    itkSetMacro(DerivativeStepLength, double);
    itkSetMacro(Iterations, unsigned int);
    itkSetMacro(Scales, ::itk::LevenbergMarquardtOptimizer::ScalesType);

    itkGetMacro(Epsilon, double);
    itkGetMacro(GradientTolerance, double);
    itkGetMacro(ValueTolerance, double);
    itkGetMacro(DerivativeStepLength, double);
    itkGetMacro(Iterations, unsigned int);
    itkGetMacro(Scales, ::itk::LevenbergMarquardtOptimizer::ScalesType);

    itkSetConstObjectMacro(ConstraintChecker, ConstraintCheckerBase);
    itkGetConstObjectMacro(ConstraintChecker, ConstraintCheckerBase);
    itkSetMacro(ActivateFailureThreshold, bool);
    itkGetConstMacro(ActivateFailureThreshold, bool);

    virtual ParameterNamesType GetCriterionNames() const;

  protected:

    typedef Superclass::ParametersType ParametersType;
    typedef Superclass::SignalType SignalType;

    LevenbergMarquardtModelFitFunctor();

    ~LevenbergMarquardtModelFitFunctor();

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
    double m_Epsilon;
    double m_GradientTolerance;
    double m_ValueTolerance;
    unsigned int m_Iterations;
    double m_DerivativeStepLength;
    ::itk::LevenbergMarquardtOptimizer::ScalesType m_Scales;

    /**Constraint checker. If set it will be used by the optimization strategies to add additional constraints to the
     given cost function. */
    ConstraintCheckerBase::ConstPointer m_ConstraintChecker;
    /**If set to true and an constraint checker is set. The cost function will allways fail if the penalty of the
     checker reaches the threshold. In this case no function evaluation will be done-*/
    bool m_ActivateFailureThreshold;
  };

}


#endif // MODEL_FIT_FUNCTOR_BASE_H
