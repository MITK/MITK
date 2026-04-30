/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConstraintCheckerInterface_h
#define mitkConstraintCheckerInterface_h

#include <mitkModelBase.h>

#include <MitkModelFitExports.h>

namespace mitk
{
  /**
   * \class ConstraintCheckerInterface
   * \brief Interface for all constraint checkers used in model fitting.
   *
   * Constraint checkers evaluate parameter vectors and return penalty values
   * for constraint violations. They are used by cost function decorators to
   * enforce parameter bounds during optimization.
   *
   * \note All functions must be implemented thread-safe as they are used in a
   * multi-threaded environment.
   *
   * \sa ConstraintCheckerBase, SimpleBarrierConstraintChecker
   */
  class MITKMODELFIT_EXPORT ConstraintCheckerInterface
  {
  public:

    typedef ConstraintCheckerInterface Self;

    typedef double PenaltyValueType;
    typedef itk::Array<PenaltyValueType> PenaltyArrayType;
    typedef itk::Array<double> SignalType;
    typedef ModelBase::ParametersType ParametersType;

    /** Returns the penalties for all defined constraints.
    * \pre defined constraints must address valid parameters in the passed parameters set.
    * \post the result vector has the size given by GetNumberOfConstraints().
    */
    virtual PenaltyArrayType GetPenalties(const ParametersType &parameters) const = 0;

    /** Returns the sum of all penalties.
    * \pre defined constraints must address valid parameters in the passed parameters set.
    */
    virtual PenaltyValueType GetPenaltySum(const ParametersType &parameters) const = 0;

    /** Returns the number of defined constraints, that will be checked. Thus also the size of the penalty vector.*/
    virtual unsigned int GetNumberOfConstraints() const = 0;

    /** Returns the penalty value that indicates at least one failed constrained.*/
    virtual PenaltyValueType GetFailedConstraintValue() const = 0;

  protected:

    ConstraintCheckerInterface()
    {}

    virtual ~ConstraintCheckerInterface()
    {}

  private:
    ConstraintCheckerInterface(const ConstraintCheckerInterface& source);
    void operator=(const ConstraintCheckerInterface&);  //purposely not implemented
  };

}

#endif
