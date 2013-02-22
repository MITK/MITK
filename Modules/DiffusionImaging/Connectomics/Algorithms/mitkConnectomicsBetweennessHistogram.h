
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


#ifndef _MITK_ConnectomicsBetweennessHistogram_H
#define _MITK_ConnectomicsBetweennessHistogram_H

#include<mitkConnectomicsHistogramBase.h>

#include <boost/graph/betweenness_centrality.hpp>

namespace mitk {

    /**
  * \brief A class to calculate and store the betweenness of each node   */
  class ConnectomicsBetweennessHistogram : public mitk::ConnectomicsHistogramBase
  {

  public:

    /** Enum for different ways to calculate betweenness centrality */

    enum BetweennessCalculationMode
    {
      UnweightedUndirectedMode,
      WeightedUndirectedMode
    };

    ConnectomicsBetweennessHistogram();
    virtual ~ConnectomicsBetweennessHistogram();

    /** Set the calucaltion mode */
    void SetBetweennessCalculationMode( const BetweennessCalculationMode & );

    /** Get the calculation mode */
    BetweennessCalculationMode GetBetweennessCalculationMode();

  protected:

    /* Typedefs */
    typedef mitk::ConnectomicsNetwork::NetworkType NetworkType;
    typedef boost::graph_traits< NetworkType >::vertex_iterator IteratorType;
    typedef std::vector< double > BCMapType;

    /** @brief Creates a new histogram from the network source. */
    virtual void ComputeFromConnectomicsNetwork( ConnectomicsNetwork* source );

    /** Calculate betweenness centrality ignoring the weight of the edges */
    void CalculateUnweightedUndirectedBetweennessCentrality( NetworkType*, IteratorType, IteratorType );

    /** Calculate betweenness centrality taking into consideration the weight of the edges */
    void CalculateWeightedUndirectedBetweennessCentrality( NetworkType*, IteratorType, IteratorType );

    /** Converts the centrality map to a histogram by binning */
    void ConvertCentralityMapToHistogram();

    /** Stores which mode has been selected for betweenness centrality calculation */
    BetweennessCalculationMode m_Mode;

    /** Stores the betweenness centralities for each node */
    BCMapType m_CentralityMap;
  };

}

#endif /* _MITK_ConnectomicsBetweennessHistogram_H */
