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

#ifndef _ClusteringMetricInnerAngles
#define _ClusteringMetricInnerAngles

#include <mitkClusteringMetric.h>

namespace mitk
{

/**
* \brief Fiber clustering metric based on the angles between certain parts of the streamline */

class ClusteringMetricInnerAngles : public ClusteringMetric
{

public:

  ClusteringMetricInnerAngles(){}
  virtual ~ClusteringMetricInnerAngles(){}

  float CalculateDistance(vnl_matrix<float>& s, vnl_matrix<float>& t, bool &flipped)
  {
    int p1 = 0;
    int p2 = s.cols()/4;
    int p3 = s.cols()/2;
    int p4 = 3*s.cols()/4;
    int p5 = s.cols()-1;

    float a1_s = 0;
    float a2_s = 0;
    float a3_s = 0;
    float a1_t = 0;
    float a2_t = 0;
    float a3_t = 0;

    {
      vnl_vector<float> v1 = s.get_column(p1)-s.get_column(p2); v1.normalize();
      vnl_vector<float> v2 = s.get_column(p3)-s.get_column(p2); v2.normalize();
      a1_s = dot_product(v1,v2);
      a1_s = std::acos( a1_s ) * 180.0/itk::Math::pi;
    }

    {
      vnl_vector<float> v1 = s.get_column(p1)-s.get_column(p3); v1.normalize();
      vnl_vector<float> v2 = s.get_column(p5)-s.get_column(p3); v2.normalize();
      a2_s = dot_product(v1,v2);
      a2_s = std::acos( a2_s ) * 180.0/itk::Math::pi;
    }

    {
      vnl_vector<float> v1 = s.get_column(p3)-s.get_column(p4); v1.normalize();
      vnl_vector<float> v2 = s.get_column(p5)-s.get_column(p4); v2.normalize();
      a3_s = dot_product(v1,v2);
      a3_s = std::acos( a3_s ) * 180.0/itk::Math::pi;
    }

    //
    {
      vnl_vector<float> v1 = t.get_column(p1)-t.get_column(p2); v1.normalize();
      vnl_vector<float> v2 = t.get_column(p3)-t.get_column(p2); v2.normalize();
      a1_t = dot_product(v1,v2);
      a1_t = std::acos( a1_t ) * 180.0/itk::Math::pi;
    }

    {
      vnl_vector<float> v1 = t.get_column(p1)-t.get_column(p3); v1.normalize();
      vnl_vector<float> v2 = t.get_column(p5)-t.get_column(p3); v2.normalize();
      a2_t = dot_product(v1,v2);
      a2_t = std::acos( a2_t ) * 180.0/itk::Math::pi;
    }

    {
      vnl_vector<float> v1 = t.get_column(p3)-t.get_column(p4); v1.normalize();
      vnl_vector<float> v2 = t.get_column(p5)-t.get_column(p4); v2.normalize();
      a3_t = dot_product(v1,v2);
      a3_t = std::acos( a3_t ) * 180.0/itk::Math::pi;
    }

    float d_direct = 0;
    float d_flipped = 0;

    int inc = s.cols()/4;
    for (unsigned int i=0; i<s.cols(); i += inc)
    {
      d_direct += (s.get_column(i)-t.get_column(i)).magnitude();
      d_flipped += (s.get_column(i)-t.get_column(s.cols()-i-1)).magnitude();
    }

    if (d_direct>d_flipped)
    {
      flipped = true;
      float d1 = std::fabs(a1_s-a3_t);
      float d2 = std::fabs(a2_s-a2_t);
      float d3 = std::fabs(a3_s-a1_t);
      return m_Scale * std::max( d1, std::max(d2,d3) );
    }
    flipped = false;
    float d1 = std::fabs(a1_s-a1_t);
    float d2 = std::fabs(a2_s-a2_t);
    float d3 = std::fabs(a3_s-a3_t);
    return m_Scale * std::max( d1, std::max(d2,d3) );
  }

protected:

};

}

#endif
