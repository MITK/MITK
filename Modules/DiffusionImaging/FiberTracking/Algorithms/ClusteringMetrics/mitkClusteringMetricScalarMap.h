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

#ifndef _ClusteringMetricScalarMap
#define _ClusteringMetricScalarMap

#include <mitkClusteringMetric.h>
#include <itkImage.h>
#include <itkImageRegionConstIterator.h>
#include <itkNumericTraits.h>
#include <mitkTrackingDataHandler.h>

namespace mitk
{

/**
* \brief Fiber clustering metric based on the scalar image values along a tract */

class ClusteringMetricScalarMap : public ClusteringMetric
{

public:

  typedef itk::Image<float, 3>  ItkFloatImgType;

  ClusteringMetricScalarMap()
  {
    m_Interpolator = itk::LinearInterpolateImageFunction< ItkFloatImgType, float >::New();
    this->m_Scale = 30;
  }
  virtual ~ClusteringMetricScalarMap(){}

  float CalculateDistance(vnl_matrix<float>& s, vnl_matrix<float>& t, bool &flipped)
  {
    float d_direct = 0;
    float d_flipped = 0;
    float map_distance = 0;

    vnl_vector<float> dists_d; dists_d.set_size(s.cols());
    vnl_vector<float> dists_f; dists_f.set_size(s.cols());

    int inc = s.cols()/4;
    for (unsigned int i=0; i<s.cols(); i += inc)
    {
      d_direct += (s.get_column(i)-t.get_column(i)).magnitude();
      d_flipped += (s.get_column(i)-t.get_column(s.cols()-i-1)).magnitude();
    }

    if (d_direct>d_flipped)
    {
      flipped = true;

      for (unsigned int i=0; i<s.cols(); ++i)
      {
        itk::Point<float, 3> p;
        p[0] = s[0][i];
        p[1] = s[1][i];
        p[2] = s[2][i];
        vnl_vector<float> vals1 = GetImageValuesAtPoint(p);

        p[0] = t[0][s.cols()-i-1];
        p[1] = t[1][s.cols()-i-1];
        p[2] = t[2][s.cols()-i-1];
        vnl_vector<float> vals2 = GetImageValuesAtPoint(p);

        map_distance += (vals1-vals2).magnitude();
      }
    }
    else
    {
      flipped = false;

      for (unsigned int i=0; i<s.cols(); ++i)
      {
        itk::Point<float, 3> p;
        p[0] = s[0][i];
        p[1] = s[1][i];
        p[2] = s[2][i];
        vnl_vector<float> vals1 = GetImageValuesAtPoint(p);

        p[0] = t[0][i];
        p[1] = t[1][i];
        p[2] = t[2][i];
        vnl_vector<float> vals2 = GetImageValuesAtPoint(p);

        map_distance += (vals1-vals2).magnitude();
      }
    }

    return m_Scale*map_distance;
  }

  vnl_vector<float> GetImageValuesAtPoint(itk::Point<float, 3>& itkP)
  {
    vnl_vector<float> vals; vals.set_size(m_ScalarMaps.size());
    int c = 0;
    for (auto map : m_ScalarMaps)
    {
      m_Interpolator->SetInputImage(map);
      vals[c] = mitk::imv::GetImageValue<float>(itkP, true, m_Interpolator);
      ++c;
    }
    return vals;
  }

  void SetImages(const std::vector<ItkFloatImgType::Pointer> &Parcellations)
  {
    m_ScalarMaps = Parcellations;
  }

protected:

  std::vector< ItkFloatImgType::Pointer > m_ScalarMaps;
  itk::LinearInterpolateImageFunction< ItkFloatImgType, float >::Pointer   m_Interpolator;
};

}

#endif
