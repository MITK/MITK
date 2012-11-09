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

#ifndef mitkConnectomicsSyntheticNetworkGenerator_h
#define mitkConnectomicsSyntheticNetworkGenerator_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMacro.h>

#include "mitkCommon.h"
#include "mitkImage.h"

#include "mitkConnectomicsNetwork.h"

#include "MitkDiffusionImagingExports.h"

namespace mitk
{

    /**
  * \brief A class to generate synthetic networks */
  class MitkDiffusionImaging_EXPORT ConnectomicsSyntheticNetworkGenerator : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacro(ConnectomicsSyntheticNetworkGenerator, itk::Object);
    itkNewMacro(Self);

    /** Create Synthetic Networks */
    mitk::ConnectomicsNetwork::Pointer CreateSyntheticNetwork(int networkTypeId, int paramterOne, double parameterTwo);

    /** Return whether the last attempted network generation was a success*/
    bool WasGenerationSuccessfull();

  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsSyntheticNetworkGenerator();
    ~ConnectomicsSyntheticNetworkGenerator();

    /** Generate a default geometry for synthetic images */
    mitk::Geometry3D::Pointer GenerateDefaultGeometry();

    /** Generate a synthetic cube (regular lattice) network */
    void GenerateSyntheticCubeNetwork( mitk::ConnectomicsNetwork::Pointer network, int cubeExtent, double distance );

    /** Generate a highly heterogenic network
    *
    * This is achieved by generating a center vertex and vertices on
    * a sphere surface, which are all only connected to the center
    * vertex.                                                        */
    void GenerateSyntheticCenterToSurfaceNetwork(
      mitk::ConnectomicsNetwork::Pointer network, int numberOfPoints, double radius );

    /** Generate a random network without specific characteristics
    *
    * This is achieved by generating vertices and then deciding whether to
    * specific vertices are connected by comparing a random number to the threshold */
    void GenerateSyntheticRandomNetwork(
      mitk::ConnectomicsNetwork::Pointer network, int numberOfPoints, double threshold );

    /////////////////////// Variables ////////////////////////

    /** Store whether the network generated last was generated properly */
    bool m_LastGenerationWasSuccess;
  };

}// end namespace mitk

#endif // _mitkConnectomicsSyntheticNetworkGenerator_H_INCLUDED
