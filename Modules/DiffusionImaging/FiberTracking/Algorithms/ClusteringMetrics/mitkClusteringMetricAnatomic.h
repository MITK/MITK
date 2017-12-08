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

#ifndef _ClusteringMetricAnatomic
#define _ClusteringMetricAnatomic

#include <mitkClusteringMetric.h>
#include <itkImage.h>
#include <itkImageRegionConstIterator.h>
#include <itkNumericTraits.h>

namespace mitk
{

/**
* \brief Fiber clustering metric based on white matter parcellation histograms along the tracts (Siless et al. https://www.ncbi.nlm.nih.gov/pubmed/29100937) */

class ClusteringMetricAnatomic : public ClusteringMetric
{

public:

  typedef itk::Image<short, 3>  ItkShortImgType;

  ClusteringMetricAnatomic()
    : m_Radius(1)
  {}
  virtual ~ClusteringMetricAnatomic(){}

  float CalculateDistance(vnl_matrix<float>& s, vnl_matrix<float>& t, bool &flipped)
  {
    vnl_matrix<float> hist1; hist1.set_size( (2*m_Radius+1)*(2*m_Radius+1)*(2*m_Radius+1), m_NumLabels ); hist1.fill(0);
    vnl_matrix<float> hist2; hist2.set_size( (2*m_Radius+1)*(2*m_Radius+1)*(2*m_Radius+1), m_NumLabels ); hist2.fill(0);


    float d_direct = 0;
    float d_flipped = 0;

    vnl_vector<float> dists_d; dists_d.set_size(s.cols());
    vnl_vector<float> dists_f; dists_f.set_size(s.cols());

    for (unsigned int i=0; i<s.cols(); ++i)
    {
      itk::Point<float, 3> p;
      p[0] = s[0][i];
      p[1] = s[1][i];
      p[2] = s[2][i];
      GetHistogramAtPoint(p, hist1);

      p[0] = t[0][i];
      p[1] = t[1][i];
      p[2] = t[2][i];
      GetHistogramAtPoint(p, hist2);

      d_direct += (s.get_column(i)-t.get_column(i)).magnitude();
      d_flipped += (s.get_column(i)-t.get_column(s.cols()-i-1)).magnitude();
    }

//    float eudist = 0;
    if (d_direct>d_flipped)
    {
      flipped = true;
//      eudist = d_flipped/s.cols();
    }
    else
    {
      flipped = false;
//      eudist = d_direct/s.cols();
    }

    float label_intersection = 0;
    for (unsigned int c=0; c<hist1.cols(); ++c)
    {
      bool l1 = false;
      bool l2 = false;
      for (unsigned int r=0; r<hist1.rows(); ++r)
      {
        if (hist1[r][c]>0)
          l1 = true;
        if (hist2[r][c]>0)
          l2 = true;
        if (l1 && l2)
        {
          label_intersection += 1;
          break;
        }
      }
    }

    float similarity = 0;
    if (label_intersection>0)
    {
      hist1.normalize_rows();
      hist2.normalize_rows();
      label_intersection /= m_NumLabels;

      for (unsigned int l=0; l<hist1.rows(); ++l)
        similarity += dot_product( hist1.get_row(l), hist2.get_row(l) );
      similarity /= hist1.rows();
      similarity *= label_intersection;
    }

    if (similarity>0)
      return m_Scale*0.2/similarity;
    else
      return 9999;
  }

  void GetHistogramAtPoint(itk::Point<float, 3>& itkP, vnl_matrix<float>& hist)
  {
    int parc_idx = 0;
    for (auto parc : m_Parcellations)
    {
      int dir_idx=0;
      itk::Index<3> tmp_idx;
      itk::Index<3> idx;
      parc->TransformPhysicalPointToIndex(itkP, idx);
      if (!parc->GetLargestPossibleRegion().IsInside(idx))
        continue;

      short label = parc->GetPixel(idx);
      short S0 = label;
      hist[dir_idx][m_LabelMaps.at(parc_idx).at(label)] += 1;

      for (int x=-m_Radius; x<=m_Radius; ++x)
        for (int y=-m_Radius; y<=m_Radius; ++y)
          for (int z=-m_Radius; z<=m_Radius; ++z)
          {
            if (x==0 && y==0 && z==0)
              continue;

            ++dir_idx;

            for (int d=1; d<5; ++d)
            {
              tmp_idx[0] = idx[0] + x*d;
              tmp_idx[1] = idx[1] + y*d;
              tmp_idx[2] = idx[2] + z*d;
              if (!parc->GetLargestPossibleRegion().IsInside(tmp_idx))
                break;
              label = parc->GetPixel(tmp_idx);

              if (label!=S0)
              {
                hist[dir_idx][m_LabelMaps.at(parc_idx).at(label)] += 1;
                break;
              }
            }
          }
      ++parc_idx;
    }
  }

  void SetParcellations(const std::vector<ItkShortImgType::Pointer> &Parcellations)
  {
    m_Parcellations = Parcellations;
    m_NumLabels = 0;

    for (auto parc : m_Parcellations)
    {
      std::map< short, short > label_map;
      itk::ImageRegionConstIterator<ItkShortImgType> it(parc, parc->GetLargestPossibleRegion());
      while (!it.IsAtEnd())
      {
        if (label_map.count(it.Get())==0)
        {
          label_map.insert( std::pair< short, short >( it.Get(), m_NumLabels) );
          ++m_NumLabels;
        }
        ++it;
      }
      m_LabelMaps.push_back(label_map);
    }
  }

protected:

  std::vector< ItkShortImgType::Pointer > m_Parcellations;
  short m_NumLabels;
  int m_Radius;
  std::vector< std::map< short, short > > m_LabelMaps;
};

}

#endif
