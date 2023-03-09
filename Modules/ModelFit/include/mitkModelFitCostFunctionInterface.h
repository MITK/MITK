/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelFitCostFunctionInterface_h
#define mitkModelFitCostFunctionInterface_h

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

#endif
