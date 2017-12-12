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

#ifndef _ClusteringMetricEuStd
#define _ClusteringMetricEuStd

#include <mitkClusteringMetric.h>

namespace mitk
{

/**
* \brief Fiber clustering metric based on the euclidean distance between tracts and the corresponding standard deviation of the distances */

class ClusteringMetricEuclideanStd : public ClusteringMetric
{

public:

  ClusteringMetricEuclideanStd(){}
  virtual ~ClusteringMetricEuclideanStd(){}

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
      float d = d_flipped/s.cols();
      dists_f -= d;
      d += dists_f.magnitude();

      flipped = true;
      return m_Scale*d/2;
    }

    float d = d_direct/s.cols();
    dists_d -= d;
    d += dists_d.magnitude();

    flipped = false;
    return m_Scale*d/2;
  }

protected:

};

}

#endif
