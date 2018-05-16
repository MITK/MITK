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

#ifndef mitkConnectomicsNetworkConverter_h
#define mitkConnectomicsNetworkConverter_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMacro.h>

#include "mitkCommon.h"

#include <MitkConnectomicsExports.h>

#include <mitkConnectomicsNetwork.h>

namespace mitk
{
  /**
  * \brief A class giving functions for conversion of connectomics networks into different formats */
  class MITKCONNECTOMICS_EXPORT ConnectomicsNetworkConverter : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacroItkParent(ConnectomicsNetworkConverter, itk::Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    // Typedefs
    typedef mitk::ConnectomicsNetwork::NetworkType NetworkType;
    typedef mitk::ConnectomicsNetwork::VertexDescriptorType VertexDescriptorType;
    typedef mitk::ConnectomicsNetwork::EdgeDescriptorType EdgeDescriptorType;
    typedef boost::graph_traits<NetworkType>::vertex_iterator VertexIteratorType;
    typedef boost::graph_traits<NetworkType>::adjacency_iterator AdjacencyIteratorType;
    typedef std::map<EdgeDescriptorType, int> EdgeIndexStdMapType;
    typedef boost::associative_property_map< EdgeIndexStdMapType > EdgeIndexMapType;
    typedef boost::iterator_property_map< std::vector< double >::iterator, EdgeIndexMapType > EdgeIteratorPropertyMapType;
    typedef boost::property_map< NetworkType, boost::vertex_index_t>::type VertexIndexMapType;
    typedef boost::iterator_property_map< std::vector< double >::iterator, VertexIndexMapType > VertexIteratorPropertyMapType;

    //Macro
    itkSetObjectMacro( Network, mitk::ConnectomicsNetwork );

    // Conversion Getters
    vnl_matrix<double> GetNetworkAsVNLAdjacencyMatrix();
    vnl_matrix<double> GetNetworkAsVNLDegreeMatrix();

  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsNetworkConverter();
    ~ConnectomicsNetworkConverter() override;

    /////////////////////// Variables ////////////////////////
    // The connectomics network, which is converted
    mitk::ConnectomicsNetwork::Pointer m_Network;
  };

}// end namespace mitk

#endif // mitkConnectomicsNetworkConverter_h
