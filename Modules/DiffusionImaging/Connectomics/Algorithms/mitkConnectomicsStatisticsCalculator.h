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

#ifndef mitkConnectomicsStatisticsCalculator_h
#define mitkConnectomicsStatisticsCalculator_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMacro.h>

#include "mitkCommon.h"

#include "ConnectomicsExports.h"

#include <mitkConnectomicsNetwork.h>

namespace mitk
{
  /**
  * \brief A class giving functions for calculating a variety of network indices */
  class Connectomics_EXPORT ConnectomicsStatisticsCalculator : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacro(ConnectomicsStatisticsCalculator, itk::Object);
    itkNewMacro(Self);

    // Typedefs
    typedef mitk::ConnectomicsNetwork::NetworkType NetworkType;
    typedef mitk::ConnectomicsNetwork::VertexDescriptorType VertexDescriptorType;
    typedef mitk::ConnectomicsNetwork::EdgeDescriptorType EdgeDescriptorType;
    typedef boost::graph_traits<NetworkType>::vertex_iterator VertexIteratorType;

    // Set/Get Macros
    itkSetObjectMacro( Network, mitk::ConnectomicsNetwork );
    itkGetMacro( NumberOfVertices, unsigned int );
    itkGetMacro( NumberOfEdges, unsigned int );
    itkGetMacro( AverageDegree, double );
    itkGetMacro( ConnectionDensity, double );
    itkGetMacro( NumberOfConnectedComponents, unsigned int );
    itkGetMacro( AverageComponentSize, double );
    itkGetMacro( Components, std::vector< int > );
    itkGetMacro( LargestComponentSize, unsigned int );
    itkGetMacro( RatioOfNodesInLargestComponent, double );
    itkGetMacro( HopPlotExponent, double );
    itkGetMacro( EffectiveHopDiameter, double );

    void Update();

  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsStatisticsCalculator();
    ~ConnectomicsStatisticsCalculator();

    void CalculateNumberOfVertices();

    void CalculateNumberOfEdges();

    void CalculateAverageDegree();

    void CalculateConnectionDensity();

    void CalculateNumberOfConnectedComponents();

    void CalculateAverageComponentSize();

    void CalculateLargestComponentSize();

    void CalculateRatioOfNodesInLargestComponent();

    void CalculateHopPlotValues();

    /////////////////////// Variables ////////////////////////

    // The connectomics network, which is used for statistics calculation
    mitk::ConnectomicsNetwork::Pointer m_Network;

    // Statistics
    unsigned int m_NumberOfVertices;
    unsigned int m_NumberOfEdges;
    double m_AverageDegree;
    double m_ConnectionDensity;
    unsigned int m_NumberOfConnectedComponents;
    double m_AverageComponentSize;
    std::vector< int > m_Components;
    unsigned int m_LargestComponentSize;
    double m_RatioOfNodesInLargestComponent;
    double m_HopPlotExponent;
    double m_EffectiveHopDiameter;
  };

}// end namespace mitk

#endif // mitkConnectomicsStatisticsCalculator_h
