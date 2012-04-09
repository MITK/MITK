/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef mitkConnectomicsSimulatedAnnealingCostFunctionBase_h
#define mitkConnectomicsSimulatedAnnealingCostFunctionBase_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMacro.h>

#include "mitkCommon.h"

#include "MitkDiffusionImagingExports.h"

namespace mitk
{
  /**
  * \brief A generic base class for cost functions for use in simulated annealing */
  class MitkDiffusionImaging_EXPORT ConnectomicsSimulatedAnnealingCostFunctionBase : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacro(ConnectomicsSimulatedAnnealingCostFunctionBase, itk::Object);
    itkNewMacro(Self);



  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsSimulatedAnnealingCostFunctionBase();
    ~ConnectomicsSimulatedAnnealingCostFunctionBase();

  };

}// end namespace mitk

#endif // mitkConnectomicsSimulatedAnnealingCostFunctionBase_h