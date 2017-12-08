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

#ifndef _ClusteringMetricEuMean
#define _ClusteringMetricEuMean

#include <mitkClusteringMetric.h>

namespace mitk
{

/**
* \brief Fiber clustering metric based on the mean euclidean distance between tracts */

class ClusteringMetricEuclideanMean : public ClusteringMetric
{

public:

  ClusteringMetricEuclideanMean(){}
  virtual ~ClusteringMetricEuclideanMean(){}

  float CalculateDistance(vnl_matrix<float>& s, vnl_matrix<float>& t, bool &flipped)
  {
    float d_direct = 0;
    float d_flipped = 0;

    for (unsigned int i=0; i<s.cols(); ++i)
    {
      d_direct += (s.get_column(i)-t.get_column(i)).magnitude();
      d_flipped += (s.get_column(i)-t.get_column(s.cols()-i-1)).magnitude();
    }

    if (d_direct>d_flipped)
    {
      flipped = true;
      return m_Scale*d_flipped/s.cols();
    }
    flipped = false;
    return m_Scale*d_direct/s.cols();
  }

protected:

};

}

#endif
