/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkConstraintCheckerBase_h
#define mitkConstraintCheckerBase_h

#include <itkObject.h>
#include <itkMacro.h>

#include <mitkConstraintCheckerInterface.h>

#include <MitkModelFitExports.h>

namespace mitk
{

/**
 * \class ConstraintCheckerBase
 * \brief Base class for constraint checkers used in model fitting.
 *
 * Provides a default implementation of GetPenaltySum() that sums all penalty values.
 *
 * \remark All functions of the ConstraintCheckerInterface must be implemented thread-safe
 * because they are used in a multi-threaded environment.
 *
 * \sa ConstraintCheckerInterface, SimpleBarrierConstraintChecker
 */
class MITKMODELFIT_EXPORT ConstraintCheckerBase : public itk::Object, public ConstraintCheckerInterface
{
public:

    typedef ConstraintCheckerBase Self;
    typedef ConstraintCheckerInterface Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    typedef Superclass::PenaltyValueType PenaltyValueType;
    typedef Superclass::PenaltyArrayType PenaltyArrayType;
    typedef Superclass::SignalType SignalType;
    typedef Superclass::ParametersType ParametersType;

    PenaltyValueType GetPenaltySum(const ParametersType &parameters) const override;

protected:

    ConstraintCheckerBase()
    {
    }

    ~ConstraintCheckerBase() override{}

private:
    ConstraintCheckerBase(const ConstraintCheckerBase& source);
    void operator=(const ConstraintCheckerBase&);  //purposely not implemented
};

}

#endif
