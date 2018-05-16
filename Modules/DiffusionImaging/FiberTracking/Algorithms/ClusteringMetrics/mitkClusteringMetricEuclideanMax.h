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

#ifndef _ClusteringMetricEuMax
#define _ClusteringMetricEuMax

#include <mitkClusteringMetric.h>

namespace mitk
{

/**
* \brief Fiber clustering metric based on the euclidean maximum distance between tracts */

class ClusteringMetricEuclideanMax : public ClusteringMetric
{

public:

  ClusteringMetricEuclideanMax(){}
  virtual ~ClusteringMetricEuclideanMax(){}

  float CalculateDistance(vnl_matrix<float>& s, vnl_matrix<float>& t, bool &flipped)
  {
    float d_direct = 0;
    float d_flipped = 0;

    vnl_vector<float> dists_d; dists_d.set_size(s.cols());
    vnl_vector<float> dists_f; dists_f.set_size(s.cols());

    for (unsigned int i=0; i<s.cols(); ++i)
    {
      dists_d[i] = (s.get_column(i)-t.get_column(i)).magnitude();
      d_direct += dists_d[i];

      dists_f[i] = (s.get_column(i)-t.get_column(s.cols()-i-1)).magnitude();
      d_flipped += dists_f[i];
    }

    if (d_direct>d_flipped)
    {
      float d = dists_f.max_value();
      flipped = true;
      return m_Scale*d;
    }

    float d = dists_d.max_value();
    flipped = false;
    return m_Scale*d;
  }

protected:

};

}

#endif
