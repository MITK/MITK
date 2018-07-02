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

#ifndef MODELFITCOSTFUNCTION_INTERFACE_H
#define MODELFITCOSTFUNCTION_INTERFACE_H

#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{
/** Interface class that is implemented by all cost functions that are used in the context of model fitting.
*/
class MITKMODELFIT_EXPORT ModelFitCostFunctionInterface
{
public:

    typedef ModelFitCostFunctionInterface Self;

    typedef itk::Array<double> SignalType;

    virtual void SetSample(const SignalType &sampleSet) = 0;

    virtual void SetModel(const ModelBase* model) = 0;
    virtual const ModelBase* GetModel() const = 0;

protected:

    ModelFitCostFunctionInterface()
    {
    }

    virtual ~ModelFitCostFunctionInterface()
    {}

private:

    ModelFitCostFunctionInterface(const ModelFitCostFunctionInterface& source);
    void operator=(const ModelFitCostFunctionInterface&);  //purposely not implemented

};

}

#endif // MODELFITCOSTFUNCTION_H
