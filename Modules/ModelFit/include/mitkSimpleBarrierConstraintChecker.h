/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef SIMPLE_BARRIER_CONSTRAINT_CHECKER_H
#define SIMPLE_BARRIER_CONSTRAINT_CHECKER_H

#include "mitkConstraintCheckerBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  /** \class SimpleBarrierConstraintChecker
   * \brief This class implements constraints as simple barrier functions.
   *
   * SimpleBarrierConstraintChecker is used to check parameters against inequality
   * constraints realized as simple barrier functions.\n
   * Inequality constraints will be transformed to an inequality greater
   * or equal zero (c(x)>= 0). The BarrierSize indicates the size of the
   * region in front of the border of the legal value range. The barrier
   * function is modelled by log(c(x)/BarrierSize).\n
   * Example:\n
   * The given constraint is 500<= param_1. Its barriersize is 50, so
   * the barrier region is between 500 and 550.\n
   * The constraint will be transformed in 0 <= param_1 - 500.
   * The barrier function will 0 for any param_1 > 550 and
   * min(-log((param_1 - 500)/50), MaxConstraintPenalty)
   * for any other value of param_1.\n
   * If you have set a sum barrier, it will be checked against the value sum of the
   * specified parameters.
   */
  class MITKMODELFIT_EXPORT SimpleBarrierConstraintChecker : public ConstraintCheckerBase
  {
  public:
    typedef SimpleBarrierConstraintChecker Self;
    typedef ConstraintCheckerBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkFactorylessNewMacro(Self);

    typedef Superclass::PenaltyValueType PenaltyValueType;
    typedef Superclass::PenaltyArrayType PenaltyArrayType;
    typedef Superclass::SignalType SignalType;
    typedef Superclass::ParametersType ParametersType;

    typedef double BarrierValueType;
    typedef double BarrierWidthType;
    typedef ParametersType::SizeValueType ParameterIndexType;
    typedef std::vector<ParameterIndexType> ParameterIndexVectorType;

    struct Constraint
    {
      ParameterIndexVectorType parameters;
      BarrierValueType barrier;
      BarrierWidthType width;
      bool upperBarrier;
      Constraint(): barrier(0), width(0), upperBarrier(true)
      {};
    };

    PenaltyArrayType GetPenalties(const ParametersType& parameters) const override;

    unsigned int GetNumberOfConstraints() const override;

    PenaltyValueType GetFailedConstraintValue() const override;

    /** Sets a lower barrier for one parameter*/
    void SetLowerBarrier(ParameterIndexType parameterID, BarrierValueType barrier,
                         BarrierWidthType width = 0.0);
    /** Sets an upper barrier for one parameter*/
    void SetUpperBarrier(ParameterIndexType parameterID, BarrierValueType barrier,
                         BarrierWidthType width = 0.0);
    /** Sets a lower sum barrier for a set of parameter*/
    void SetLowerSumBarrier(const ParameterIndexVectorType& parameterIDs, BarrierValueType barrier,
                            BarrierWidthType width = 0.0);
    /** Sets an upper sum barrier for a set of parameter*/
    void SetUpperSumBarrier(const ParameterIndexVectorType& parameterIDs, BarrierValueType barrier,
                            BarrierWidthType width = 0.0);

    /*returns the constraint with the given index.
     @pre The index must exist.*/
    Constraint& GetConstraint(unsigned int index);
    /*returns the constraint with the given index.
     @pre The index must exist.*/
    const Constraint& GetConstraint(unsigned int index) const;

    /*removes a constraint. Indicated by the index. If the index does not exist,
     nothing will be removed and the state of the checker stays untouched.*/
    void DeleteConstraint(unsigned int index);

    void ResetConstraints();

    itkSetMacro(MaxConstraintPenalty, PenaltyValueType);
    itkGetConstMacro(MaxConstraintPenalty, PenaltyValueType);

  protected:

    typedef std::vector<Constraint> ConstraintVectorType;
    ConstraintVectorType m_Constraints;

    PenaltyValueType CalcPenalty(const ParametersType& parameters, const Constraint& constraint) const;

    SimpleBarrierConstraintChecker() : m_MaxConstraintPenalty(1e15)
    {
    }

    ~SimpleBarrierConstraintChecker() override {}

  private:
    PenaltyValueType m_MaxConstraintPenalty;
  };

}

#endif // SimpleBarrierConstraintChecker_H
