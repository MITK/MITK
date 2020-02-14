/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSimpleBarrierConstraintChecker.h"

#include <algorithm>

#include "mitkExceptionMacro.h"

mitk::SimpleBarrierConstraintChecker::PenaltyArrayType
mitk::SimpleBarrierConstraintChecker::GetPenalties(const ParametersType& parameters) const
{
  PenaltyArrayType result(m_Constraints.size());

  PenaltyArrayType::iterator penaltyPos = result.begin();

  for (ConstraintVectorType::const_iterator pos = m_Constraints.begin(); pos != m_Constraints.end();
       ++pos, ++penaltyPos)
  {
    *penaltyPos = CalcPenalty(parameters, *pos);
  }

  return result;
};

unsigned int mitk::SimpleBarrierConstraintChecker::GetNumberOfConstraints() const
{
  return m_Constraints.size();
};

mitk::SimpleBarrierConstraintChecker::PenaltyValueType
mitk::SimpleBarrierConstraintChecker::GetFailedConstraintValue() const
{
  return m_MaxConstraintPenalty;
};

void mitk::SimpleBarrierConstraintChecker::SetLowerBarrier(ParameterIndexType parameterID,
    BarrierValueType barrier, BarrierWidthType width)
{
  Constraint c;
  ParameterIndexVectorType params;
  params.push_back(parameterID);
  c.parameters = params;
  c.barrier = barrier;
  c.width = width;
  c.upperBarrier = false;

  m_Constraints.push_back(c);
};

void mitk::SimpleBarrierConstraintChecker::SetUpperBarrier(ParameterIndexType parameterID,
    BarrierValueType barrier, BarrierWidthType width)
{
  Constraint c;
  ParameterIndexVectorType params;
  params.push_back(parameterID);
  c.parameters = params;
  c.barrier = barrier;
  c.width = width;
  c.upperBarrier = true;

  m_Constraints.push_back(c);
};

void mitk::SimpleBarrierConstraintChecker::SetLowerSumBarrier(const ParameterIndexVectorType&
    parameterIDs, BarrierValueType barrier, BarrierWidthType width)
{
  Constraint c;

  c.parameters = parameterIDs;
  c.barrier = barrier;
  c.width = width;
  c.upperBarrier = false;

  m_Constraints.push_back(c);
};

void mitk::SimpleBarrierConstraintChecker::SetUpperSumBarrier(const ParameterIndexVectorType&
    parameterIDs, BarrierValueType barrier, BarrierWidthType width)
{
  Constraint c;

  c.parameters = parameterIDs;
  c.barrier = barrier;
  c.width = width;
  c.upperBarrier = true;

  m_Constraints.push_back(c);
};

mitk::SimpleBarrierConstraintChecker::Constraint&
mitk::SimpleBarrierConstraintChecker::GetConstraint(unsigned int index)
{
  if (index >= GetNumberOfConstraints())
  {
    mitkThrow() <<
                "Error. Cannot get constraint. Invalid constraint index passed.Parameter Invalid index:"
                << index;
  }

  return m_Constraints[index];
};

const mitk::SimpleBarrierConstraintChecker::Constraint&
mitk::SimpleBarrierConstraintChecker::GetConstraint(unsigned int index) const
{
  if (index >= GetNumberOfConstraints())
  {
    mitkThrow() <<
                "Error. Cannot get constraint. Invalid constraint index passed.Parameter Invalid index:"
                << index;
  }

  return m_Constraints[index];
};

void mitk::SimpleBarrierConstraintChecker::DeleteConstraint(unsigned int index)
{
  if (index >= GetNumberOfConstraints())
  {
    return;
  }

  m_Constraints.erase(m_Constraints.begin() + index);
};


void mitk::SimpleBarrierConstraintChecker::ResetConstraints()
{
  m_Constraints.clear();
};

mitk::SimpleBarrierConstraintChecker::PenaltyValueType
mitk::SimpleBarrierConstraintChecker::CalcPenalty(const ParametersType& parameters,
    const Constraint& constraint) const
{
  double constrainedValue = 0;

  for (ParameterIndexVectorType::const_iterator pos = constraint.parameters.begin();
       pos != constraint.parameters.end(); ++pos)
  {
    if (*pos >= parameters.size())
    {
      mitkThrow() <<
                  "Error. Parameter specified by constraint is no part of the passed parameter values. Invalid parameter ID:"
                  << *pos;
    }

    constrainedValue += parameters[*pos];
  };

  PenaltyValueType result = 0;

  //transform into an inequality against 0.
  //basic form of inequalities are constrainedValue (relation) constraint.barrier
  double transformedTermValue = constraint.barrier - constrainedValue;

  if (!constraint.upperBarrier)
  {
    // need the form 0 <= term, but right now it would be 0 >= term, so *-1
    transformedTermValue *= -1;
  }

  double barrierSize = std::abs(static_cast<double>(constraint.width));

  //Check if it would be the maximum penalty anyway.
  if (transformedTermValue <= 0)
  {
    result = this->m_MaxConstraintPenalty;
  }
  else if (transformedTermValue > barrierSize)
  {
    //Check if it is outside the barrier region
    //the constrained value is outside the barrier region within a legal valid range.
    result = 0;
  }
  else
  {
    // the constrained value seems to violate the constraint or at least is within the
    // barrier region, so compute the penalty

    //calculate the barrier function value
    double barrierValue = -1 * log(transformedTermValue / barrierSize);

    result = std::min(barrierValue, this->m_MaxConstraintPenalty);
  }

  return result;
};
