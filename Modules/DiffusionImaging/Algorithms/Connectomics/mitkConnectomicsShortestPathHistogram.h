
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


#ifndef _MITK_ConnectomicsShortestPathHistogram_H
#define _MITK_ConnectomicsShortestPathHistogram_H

#include<mitkConnectomicsHistogramBase.h>

#include "MitkDiffusionImagingExports.h"

namespace mitk {

    /**
  * \brief A class to calculate and store the shortest path between each pair of nodes */
  class MitkDiffusionImaging_EXPORT ConnectomicsShortestPathHistogram : public mitk::ConnectomicsHistogramBase
  {

  public:

    /** Enum for different ways to calculate shortest paths */
    enum ShortestPathCalculationMode
    {
      UnweightedUndirectedMode,
      WeightedUndirectedMode
    };

    ConnectomicsShortestPathHistogram();
    virtual ~ConnectomicsShortestPathHistogram();

    /** Set the calucaltion mode */
    void SetShortestPathCalculationMode( const ShortestPathCalculationMode & );

    /** Get the calculation mode */
    ShortestPathCalculationMode GetShortestPathCalculationMode();

    /** Get efficiency */
    double GetEfficiency();


  protected:

    /* Typedefs */
    typedef mitk::ConnectomicsNetwork::NetworkType NetworkType;
    typedef boost::graph_traits< NetworkType >::vertex_descriptor DescriptorType;
    typedef boost::graph_traits< NetworkType >::vertex_iterator IteratorType;

    /** @brief Creates a new histogram from the network source. */
    virtual void ComputeFromConnectomicsNetwork( ConnectomicsNetwork* source );

    /** Calculate shortest paths ignoring the weight of the edges */
    void CalculateUnweightedUndirectedShortestPaths( NetworkType* boostGraph );

    /** Calculate shortest paths taking into consideration the weight of the edges */
    void CalculateWeightedUndirectedShortestPaths( NetworkType* boostGraph );

    /** Converts the distance map to a histogram */
    void ConvertDistanceMapToHistogram();

    /** Stores which mode has been selected for shortest path calculation */
    ShortestPathCalculationMode m_Mode;

    /** Stores the shortest paths between the nodes */
    std::vector< std::vector< int > > m_DistanceMatrix;

    /** Stores, whether the graph has disconnected components  */
    bool m_EverythingConnected;
  };

}

#endif /* _MITK_ConnectomicsShortestPathHistogram_H */
