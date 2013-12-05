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

#include "ConnectomicsExports.h"

#include <mitkConnectomicsNetwork.h>

namespace mitk
{
  /**
  * \brief A class for thresholding connectomics networks */
  class Connectomics_EXPORT ConnectomicsNetworkThresholder : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacro(ConnectomicsNetworkThresholder, itk::Object);
    itkNewMacro(Self);

    // Typedefs
    typedef mitk::ConnectomicsNetwork::NetworkType NetworkType;
    typedef mitk::ConnectomicsNetwork::VertexDescriptorType VertexDescriptorType;
    typedef mitk::ConnectomicsNetwork::EdgeDescriptorType EdgeDescriptorType;
    typedef boost::graph_traits<NetworkType>::vertex_iterator VertexIteratorType;
    typedef boost::graph_traits<NetworkType>::edge_iterator EdgeIteratorType;
    typedef boost::graph_traits<NetworkType>::adjacency_iterator AdjacencyIteratorType;
    typedef std::map<EdgeDescriptorType, int> EdgeIndexStdMapType;
    typedef boost::associative_property_map< EdgeIndexStdMapType > EdgeIndexMapType;
    typedef boost::iterator_property_map< std::vector< double >::iterator, EdgeIndexMapType > EdgeIteratorPropertyMapType;
    typedef boost::property_map< NetworkType, boost::vertex_index_t>::type VertexIndexMapType;
    typedef boost::iterator_property_map< std::vector< double >::iterator, VertexIndexMapType > VertexIteratorPropertyMapType;

    // Set/Get Macros
    itkSetObjectMacro( Network, mitk::ConnectomicsNetwork );

    void Update();

  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsNetworkThresholder();
    ~ConnectomicsNetworkThresholder();


    /////////////////////// Variables ////////////////////////

    // The connectomics network, which is used for statistics calculation
    mitk::ConnectomicsNetwork::Pointer m_Network;

  };

}// end namespace mitk

#endif // mitkConnectomicsNetworkThresholder_h
