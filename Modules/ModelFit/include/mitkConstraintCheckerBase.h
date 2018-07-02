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

#ifndef CONSTRAINT_CHECKER_BASE_H
#define CONSTRAINT_CHECKER_BASE_H

#include <itkObject.h>
#include <itkMacro.h>

#include "mitkConstraintCheckerInterface.h"

#include "MitkModelFitExports.h"

namespace mitk
{

/** \class ConstraintCheckerBase
 * \brief This class is the base class for constraint checker.
 * @remark All functions of the ConstraintCheckerInterface must be implemented thread save because it will be used in a multi threaded
 * environment.
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

    virtual PenaltyValueType GetPenaltySum(const ParametersType &parameters) const;

protected:

    ConstraintCheckerBase()
    {
    }

    ~ConstraintCheckerBase(){}

private:
    ConstraintCheckerBase(const ConstraintCheckerBase& source);
    void operator=(const ConstraintCheckerBase&);  //purposely not implemented
};

}

#endif // ConstraintCheckerBase_H
