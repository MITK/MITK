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

#ifndef mitkConnectomicsSimulatedAnnealingCostFunctionBase_h
#define mitkConnectomicsSimulatedAnnealingCostFunctionBase_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMacro.h>

#include "mitkCommon.h"

#include <MitkConnectomicsExports.h>

namespace mitk
{
  /**
  * \brief A generic base class for cost functions for use in simulated annealing */
  class MITKCONNECTOMICS_EXPORT ConnectomicsSimulatedAnnealingCostFunctionBase : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacroItkParent(ConnectomicsSimulatedAnnealingCostFunctionBase, itk::Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)



  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsSimulatedAnnealingCostFunctionBase();
    ~ConnectomicsSimulatedAnnealingCostFunctionBase() override;

  };

}// end namespace mitk

#endif // mitkConnectomicsSimulatedAnnealingCostFunctionBase_h
