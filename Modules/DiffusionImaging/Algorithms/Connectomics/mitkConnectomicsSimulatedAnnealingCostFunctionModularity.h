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

#ifndef mitkConnectomicsSimulatedAnnealingCostFunctionModularity_h
#define mitkConnectomicsSimulatedAnnealingCostFunctionModularity_h

#include "mitkConnectomicsSimulatedAnnealingCostFunctionBase.h"

#include "mitkConnectomicsNetwork.h"

namespace mitk
{
  /**
  * \brief A cost function using the modularity of the network */
  class MitkDiffusionImaging_EXPORT ConnectomicsSimulatedAnnealingCostFunctionModularity : public mitk::ConnectomicsSimulatedAnnealingCostFunctionBase
  {
  public:

    typedef mitk::ConnectomicsNetwork::VertexDescriptorType VertexDescriptorType;
    typedef std::map< VertexDescriptorType, int > ToModuleMapType;
    typedef std::map< VertexDescriptorType, VertexDescriptorType > VertexToVertexMapType;

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacro(ConnectomicsSimulatedAnnealingCostFunctionModularity, itk::Object);
    itkNewMacro(Self);

    // Evaluate the network according to the set cost function
    double Evaluate( mitk::ConnectomicsNetwork::Pointer network, ToModuleMapType *vertexToModuleMap  ) const;

    // Will calculate and return the modularity of the network
    double CalculateModularity( mitk::ConnectomicsNetwork::Pointer network, ToModuleMapType *vertexToModuleMap  ) const;


  protected:

    // returns the number of modules
    int getNumberOfModules( ToModuleMapType *vertexToModuleMap ) const;

    //////////////////// Functions ///////////////////////
    ConnectomicsSimulatedAnnealingCostFunctionModularity();
    ~ConnectomicsSimulatedAnnealingCostFunctionModularity();

  };

}// end namespace mitk

#endif // mitkConnectomicsSimulatedAnnealingCostFunctionModularity_h