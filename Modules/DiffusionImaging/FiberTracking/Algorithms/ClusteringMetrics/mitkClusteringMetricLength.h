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

#ifndef _ClusteringMetricLength
#define _ClusteringMetricLength

#include <mitkClusteringMetric.h>

namespace mitk
{

/**
* \brief Fiber clustering metric based on the angles between certain parts of the streamline */

class ClusteringMetricLength : public ClusteringMetric
{

public:

  ClusteringMetricLength(){}
  virtual ~ClusteringMetricLength(){}

  float CalculateDistance(vnl_matrix<float>& s, vnl_matrix<float>& t, bool &flipped)
  {
    float l1 = 0;
    float l2 = 0;

    float d_direct = 0;
    float d_flipped = 0;

    int inc = s.cols()/4;
    for (unsigned int i=0; i<s.cols(); i += inc)
    {
      d_direct += (s.get_column(i)-t.get_column(i)).magnitude();
      d_flipped += (s.get_column(i)-t.get_column(s.cols()-i-1)).magnitude();

      if (i<s.cols()-1)
      {
        l1 += (s.get_column(i+1)-s.get_column(i)).magnitude();
        l2 += (t.get_column(i+1)-t.get_column(i)).magnitude();
      }
    }

    if (d_direct>d_flipped)
    {
      flipped = true;
      return m_Scale * std::fabs(l1-l2);
    }
    flipped = false;
    return m_Scale * std::fabs(l1-l2);
  }

protected:

};

}

#endif
