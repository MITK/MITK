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

#ifndef mitkConnectomicsNetworkThresholder_h
#define mitkConnectomicsNetworkThresholder_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMacro.h>

#include "mitkCommon.h"

#include <MitkConnectomicsExports.h>

#include <mitkConnectomicsNetwork.h>

namespace mitk
{
  /**
  * \brief A class for thresholding connectomics networks */
  class MITKCONNECTOMICS_EXPORT ConnectomicsNetworkThresholder : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacroItkParent(ConnectomicsNetworkThresholder, itk::Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    // Typedefs
    typedef mitk::ConnectomicsNetwork::NetworkType NetworkType;
    typedef mitk::ConnectomicsNetwork::VertexDescriptorType VertexDescriptorType;
    typedef mitk::ConnectomicsNetwork::EdgeDescriptorType EdgeDescriptorType;
    typedef boost::graph_traits<NetworkType>::vertex_iterator VertexIteratorType;
    typedef boost::graph_traits<NetworkType>::edge_iterator EdgeIteratorType;
    typedef boost::graph_traits<NetworkType>::adjacency_iterator AdjacencyIteratorType;
    typedef std::map<EdgeDescriptorType, int> EdgeIndexStdMapType;
    typedef boost::associative_property_map< EdgeIndexStdMapType > EdgeIndexMapType;
    typedef std::map<VertexDescriptorType, int> VertexIndexStdMapType;
    typedef boost::associative_property_map< VertexIndexStdMapType > VertexIndexMapType;

    /** \brief Possible schemes for thresholding the network
    *
    * \li \c RandomRemovalOfWeakest: Randomly remove one of the weakest edges until a given density is reached
    * \li \c LargestLowerThanDensity: Remove all edges of the lowest weight, repeat until below the specified density
    * \li \c ThresholdBased: Remove all edges with a weight of threshold and lower. This will not take density information into account.
    */
    enum ThresholdingSchemes
    {
      RandomRemovalOfWeakest,
      LargestLowerThanDensity,
      ThresholdBased
    };

    // Set/Get Macros
    itkSetObjectMacro( Network, mitk::ConnectomicsNetwork );
    itkSetMacro( ThresholdingScheme, ThresholdingSchemes );
    itkSetMacro( TargetThreshold, double );
    itkSetMacro( TargetDensity, double );

    itkGetObjectMacro( Network, mitk::ConnectomicsNetwork );
    itkGetMacro( ThresholdingScheme, ThresholdingSchemes );
    itkGetMacro( TargetThreshold, double );
    itkGetMacro( TargetDensity, double );

    /** \brief Apply thresholding scheme and get resulting network */
    mitk::ConnectomicsNetwork::Pointer GetThresholdedNetwork();

  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsNetworkThresholder();
    ~ConnectomicsNetworkThresholder() override;

    mitk::ConnectomicsNetwork::Pointer ThresholdByRandomRemoval( mitk::ConnectomicsNetwork::Pointer input, double targetDensity );
    mitk::ConnectomicsNetwork::Pointer ThresholdBelowDensity( mitk::ConnectomicsNetwork::Pointer input, double targetDensity );
    mitk::ConnectomicsNetwork::Pointer Threshold( mitk::ConnectomicsNetwork::Pointer input, double targetThreshold );
    // Returns false if parameter combination is invalid
    bool CheckValidity();

    /////////////////////// Variables ////////////////////////

    // The connectomics network, which is used for statistics calculation
    mitk::ConnectomicsNetwork::Pointer m_Network;

    // The thresholding scheme to be used
    ThresholdingSchemes m_ThresholdingScheme;

    double m_TargetThreshold;
    double m_TargetDensity;

  };

}// end namespace mitk

#endif // mitkConnectomicsNetworkThresholder_h
