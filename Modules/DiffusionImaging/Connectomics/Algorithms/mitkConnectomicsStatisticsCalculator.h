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

#include <MitkConnectomicsExports.h>

#include <mitkConnectomicsNetwork.h>

namespace mitk
{
  /**
  * \brief A class giving functions for calculating a variety of network indices */
  class MITKCONNECTOMICS_EXPORT ConnectomicsStatisticsCalculator : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacroItkParent(ConnectomicsStatisticsCalculator, itk::Object);
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
    typedef boost::iterator_property_map< std::vector< double >::iterator, EdgeIndexMapType > EdgeIteratorPropertyMapType;
    typedef boost::property_map< NetworkType, boost::vertex_index_t>::type VertexIndexMapType;
    typedef boost::iterator_property_map< std::vector< double >::iterator, VertexIndexMapType > VertexIteratorPropertyMapType;

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
    itkGetMacro( VectorOfClusteringCoefficientsC, std::vector< double > );
    itkGetMacro( VectorOfClusteringCoefficientsD, std::vector< double > );
    itkGetMacro( VectorOfClusteringCoefficientsE, std::vector< double > );
    itkGetMacro( AverageClusteringCoefficientsC, double );
    itkGetMacro( AverageClusteringCoefficientsD, double );
    itkGetMacro( AverageClusteringCoefficientsE, double );
    itkGetMacro( VectorOfVertexBetweennessCentralities, std::vector< double > );
    itkGetMacro( PropertyMapOfVertexBetweennessCentralities, VertexIteratorPropertyMapType );
    itkGetMacro( AverageVertexBetweennessCentrality, double );
    itkGetMacro( VectorOfEdgeBetweennessCentralities, std::vector< double > );
    itkGetMacro( PropertyMapOfEdgeBetweennessCentralities, EdgeIteratorPropertyMapType );
    itkGetMacro( AverageEdgeBetweennessCentrality, double );
    itkGetMacro( NumberOfIsolatedPoints, unsigned int );
    itkGetMacro( RatioOfIsolatedPoints, double );
    itkGetMacro( NumberOfEndPoints, unsigned int );
    itkGetMacro( RatioOfEndPoints, double );
    itkGetMacro( VectorOfEccentrities, std::vector< unsigned int > );
    itkGetMacro( VectorOfEccentrities90, std::vector< unsigned int > );
    itkGetMacro( VectorOfAveragePathLengths, std::vector< double > );
    itkGetMacro( Diameter, unsigned int );
    itkGetMacro( Diameter90, unsigned int );
    itkGetMacro( Radius, unsigned int );
    itkGetMacro( Radius90, unsigned int );
    itkGetMacro( AverageEccentricity, double );
    itkGetMacro( AverageEccentricity90, double );
    itkGetMacro( AveragePathLength, double );
    itkGetMacro( NumberOfCentralPoints, unsigned int );
    itkGetMacro( RatioOfCentralPoints, double );
    itkGetMacro( VectorOfSortedEigenValues, std::vector< double > );
    itkGetMacro( SpectralRadius, double );
    itkGetMacro( SecondLargestEigenValue, double );
    itkGetMacro( AdjacencyTrace, double );
    itkGetMacro( AdjacencyEnergy, double );
    itkGetMacro( VectorOfSortedLaplacianEigenValues, std::vector< double > );
    itkGetMacro( LaplacianTrace, double );
    itkGetMacro( LaplacianEnergy, double );
    itkGetMacro( LaplacianSpectralGap, double );
    itkGetMacro( VectorOfSortedNormalizedLaplacianEigenValues, std::vector< double > );
    itkGetMacro( NormalizedLaplacianTrace, double );
    itkGetMacro( NormalizedLaplacianEnergy, double );
    itkGetMacro( NormalizedLaplacianNumberOf2s, unsigned int );
    itkGetMacro( NormalizedLaplacianNumberOf1s, unsigned int );
    itkGetMacro( NormalizedLaplacianNumberOf0s, unsigned int );
    itkGetMacro( NormalizedLaplacianLowerSlope, double );
    itkGetMacro( NormalizedLaplacianUpperSlope, double );
    itkGetMacro( SmallWorldness, double );

    void Update();

  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsStatisticsCalculator();
    ~ConnectomicsStatisticsCalculator() override;

    void CalculateNumberOfVertices();

    void CalculateNumberOfEdges();

    void CalculateAverageDegree();

    void CalculateConnectionDensity();

    void CalculateNumberOfConnectedComponents();

    void CalculateAverageComponentSize();

    void CalculateLargestComponentSize();

    void CalculateRatioOfNodesInLargestComponent();

    void CalculateHopPlotValues();

    /**
    * \brief Calculate the different clustering coefficients
    *
    * The clustering coefficient (cc) measures how strong the tendency to form cliques
    * is in the network. Groups of nodes, that are highly interconnected.
    *
    * CC C - Percentage of connections between nodes connected with the given node
    * CC D - Same as C, but including the connections with the given node
    * CC E - Same as C, but not counting isolated nodes when averaging
    */
    void CalculateClusteringCoefficients();

    void CalculateBetweennessCentrality();

    void CalculateIsolatedAndEndPoints();

    void CalculateShortestPathMetrics();

    void CalculateSpectralMetrics();

    void CalculateLaplacianMetrics();

    void CalculateNormalizedLaplacianMetrics();

    /**
     * \brief Calculate the small worldness of the network.
     *
     * This will compare the clustering coefficient and mean path length of the network
     * to an Erdos-Reny network of the same number of nodes and edges.
     */
    void CalculateSmallWorldness();

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
    std::vector< double > m_VectorOfClusteringCoefficientsC;
    std::vector< double > m_VectorOfClusteringCoefficientsD;
    std::vector< double > m_VectorOfClusteringCoefficientsE;
    double m_AverageClusteringCoefficientsC;
    double m_AverageClusteringCoefficientsD;
    double m_AverageClusteringCoefficientsE;
    std::vector< double > m_VectorOfVertexBetweennessCentralities;
    VertexIteratorPropertyMapType m_PropertyMapOfVertexBetweennessCentralities;
    double m_AverageVertexBetweennessCentrality;
    std::vector< double > m_VectorOfEdgeBetweennessCentralities;
    EdgeIteratorPropertyMapType m_PropertyMapOfEdgeBetweennessCentralities;
    double m_AverageEdgeBetweennessCentrality;
    unsigned int m_NumberOfIsolatedPoints;
    double m_RatioOfIsolatedPoints;
    unsigned int m_NumberOfEndPoints;
    double m_RatioOfEndPoints;
    std::vector< unsigned int > m_VectorOfEccentrities;
    std::vector< unsigned int > m_VectorOfEccentrities90;
    std::vector< double > m_VectorOfAveragePathLengths;
    unsigned int m_Diameter;
    unsigned int m_Diameter90;
    unsigned int m_Radius;
    unsigned int m_Radius90;
    double m_AverageEccentricity;
    double m_AverageEccentricity90;
    double m_AveragePathLength;
    unsigned int m_NumberOfCentralPoints;
    double m_RatioOfCentralPoints;
    std::vector<double> m_VectorOfSortedEigenValues;
    double m_SpectralRadius;
    double m_SecondLargestEigenValue;
    double m_AdjacencyTrace;
    double m_AdjacencyEnergy;
    std::vector<double> m_VectorOfSortedLaplacianEigenValues;
    double m_LaplacianTrace;
    double m_LaplacianEnergy;
    double m_LaplacianSpectralGap;
    std::vector<double> m_VectorOfSortedNormalizedLaplacianEigenValues;
    double m_NormalizedLaplacianTrace;
    double m_NormalizedLaplacianEnergy;
    unsigned int m_NormalizedLaplacianNumberOf2s;
    unsigned int m_NormalizedLaplacianNumberOf1s;
    unsigned int m_NormalizedLaplacianNumberOf0s;
    double m_NormalizedLaplacianLowerSlope;
    double m_NormalizedLaplacianUpperSlope;
    double m_SmallWorldness;
  };

}// end namespace mitk

#endif // mitkConnectomicsStatisticsCalculator_h
