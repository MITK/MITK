/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMVConstrainedCostFunctionDecorator_h
#define mitkMVConstrainedCostFunctionDecorator_h

#include <mitkMVModelFitCostFunction.h>
#include <mitkConstraintCheckerBase.h>

#include "MitkModelFitExports.h"

namespace mitk
{

/** \class MVConstrainedCostFunctionDecorator
 * \brief This class is used to add constraints to any multi valued model fit cost function.
 *
 * MVConstrainedCostFunctionDecorator is used to extend a given cost function
 * with the functionality to regard given constraints for the measure.
 * To add this functionality to a cost function, instantiate the decorator, set
 * the wrapped cost function (that will be extended) and set a constraint checker
 * that defines the type of constraints.\n
 * The decorator has a failure threshold. An evaluation
 * can always be accounted as a failure if the sum of penalties given by the checker
 * is greater or equal to the threshold. If the evaluation is a failure the wrapped cost function
 * will not be evaluated. Otherwise the penalty will be added to every measure of the cost function.
 */
class MITKMODELFIT_EXPORT MVConstrainedCostFunctionDecorator : public mitk::MVModelFitCostFunction
{
public:

    typedef MVConstrainedCostFunctionDecorator Self;
    typedef mitk::MVModelFitCostFunction Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkNewMacro(Self);

    typedef Superclass::SignalType SignalType;
    typedef ConstraintCheckerBase::PenaltyValueType PenaltyValueType;

    itkSetConstObjectMacro(WrappedCostFunction, MVModelFitCostFunction);
    itkGetConstObjectMacro(WrappedCostFunction, MVModelFitCostFunction);

    itkSetConstObjectMacro(ConstraintChecker, ConstraintCheckerBase);
    itkGetConstObjectMacro(ConstraintChecker, ConstraintCheckerBase);

    itkSetMacro(FailureThreshold, PenaltyValueType);
    itkGetConstMacro(FailureThreshold, PenaltyValueType);
    itkSetMacro(ActivateFailureThreshold, bool);
    itkGetConstMacro(ActivateFailureThreshold, bool);

    /**Returns the number of evaluations done by the cost function instance
      since creation.*/
    itkGetConstMacro(EvaluationCount, unsigned int);

    /**Returns the ration between evaluations that were penaltized and all evaluation since
     creation of the instance. 0.0 means no evaluation was penalized; 1.0 all evaluations were.
     (evaluations that hit the failure threshold count as penalized too.)*/
    double GetPenaltyRatio() const;
    /**Returns the ration between evaluations that where beyond the failure thershold and all evaluation since
    creation of the instance. 0.0 means no evaluation was a failure (but some may be penalized);
    1.0 all evaluations were failures.*/
    double GetFailureRatio() const;

    /**Returns the index of the first (in terms of index position) failed parameter in the last failed evaluation.*/
    ParametersType::size_type GetFailedParameter() const;
protected:

    MeasureType CalcMeasure(const ParametersType &parameters, const SignalType& signal) const override;

    MVConstrainedCostFunctionDecorator() : m_FailureThreshold(1e15), m_ActivateFailureThreshold(true),
      m_EvaluationCount(0), m_PenaltyCount(0), m_FailureCount(0), m_LastFailedParameter(-1)
    {
    }

    ~MVConstrainedCostFunctionDecorator() override{}

    ConstraintCheckerBase::ConstPointer m_ConstraintChecker;
    MVModelFitCostFunction::ConstPointer m_WrappedCostFunction;
    PenaltyValueType m_FailureThreshold;
    bool m_ActivateFailureThreshold;
    mutable unsigned int m_EvaluationCount;
    mutable unsigned int m_PenaltyCount;
    mutable unsigned int m_FailureCount;
    mutable ParametersType::size_type m_LastFailedParameter;
};

}

#endif
