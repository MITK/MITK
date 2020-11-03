/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkCooccurenceMatrixFeatureFunctor_h
#define itkCooccurenceMatrixFeatureFunctor_h

#include "itkConstNeighborhoodIterator.h"

#include <itkHistogramToTextureFeaturesFilter.h>
#include <itkHistogram.h>
#include <bitset>

/*
 * To Do:
  * - Enable normalization of GLCM
  */

namespace itk
{

namespace Functor
{

/** \brief Functor for texture feature calculation based on the Cooccurence matrix
 */

template< typename TNeighborhoodType, typename TPixelOutputType>
class NeighborhoodCooccurenceMatrix
{
public:
  // Functor defines

  typedef itk::Statistics::Histogram< double, itk::Statistics::DenseFrequencyContainer2 > HistogramType;
  typedef typename TNeighborhoodType::OffsetType                                          OffsetType;
  typedef typename HistogramType::SizeType                                                SizeType;
  typedef typename HistogramType::MeasurementVectorType                                   MeasurementVectorType;
  typedef typename itk::Statistics::HistogramToTextureFeaturesFilter<HistogramType>       HistoToFeatureFilter;

  static const unsigned int OutputCount = 8;
  typedef vnl_vector_fixed<TPixelOutputType, OutputCount>  OutputVectorType;
  typedef TNeighborhoodType                                NeighborhoodType;
  enum OutputFeatures
  {
    ENERGY, ENTROPY, CORRELATION, INERTIA, CLUSTERSHADE, CLUSTERPROMINENCE, HARALICKCORRELATION, INVERSEDIFFERENCEMOMENT
  };

  static const char * GetFeatureName(unsigned int f )
  {
    static const char * const FeatureNames[] = {"ENERGY", "ENTROPY", "CORRELATION", "INERTIA", "CLUSTERSHADE", "CLUSTERPROMINENCE", "HARALICKCORRELATION", "INVERSEDIFFERENCEMOMENT"};
    return FeatureNames[f];
  }

  enum OffsetDirection
  {
    DIR_x1_x0_x0 = 1,
    DIR_x1_x1_x0 = 2,
    DIR_x0_x1_x0 = 4,
    DIR_n1_x1_x0 = 8,
    DIR_x1_x0_x1 = 16,
    DIR_x1_x1_x1 = 32,
    DIR_x0_x1_x1 = 64,
    DIR_n1_x1_x1 = 128,
    DIR_x1_x0_n1 = 256,
    DIR_x1_x1_n1 = 512,
    DIR_x0_x1_n1 = 1024,
    DIR_n1_x1_n1 = 2048,
    DIR_x0_x0_x1 = 4096,
    ROTATION_INVARIANT = 8191
  };

  static std::map<OffsetDirection, OffsetType > CreateStdOffsets()
  {
    std::map<OffsetDirection,OffsetType> offsetMap;

    {
      itk::Offset<3> off = {{1,0,0}};
      offsetMap[DIR_x1_x0_x0] = off;
    }
    {
      itk::Offset<3> off = {{1,1,0}};
      offsetMap[DIR_x1_x1_x0] = off;
    }
    {
      itk::Offset<3> off = {{0,1,0}};
      offsetMap[DIR_x0_x1_x0] = off;
    }
    {
      itk::Offset<3> off = {{-1,1,0}};
      offsetMap[DIR_n1_x1_x0] = off;
    }
    {
      itk::Offset<3> off = {{1,0,1}};
      offsetMap[DIR_x1_x0_x1]= off;
    }
    {
      itk::Offset<3> off = {{1,1,1}};
      offsetMap[DIR_x1_x1_x1] = off;
    }
    {
      itk::Offset<3> off = {{0,1,1}};
      offsetMap[DIR_x0_x1_x1] = off;
    }
    {
      itk::Offset<3> off = {{-1,1,1}};
      offsetMap[DIR_n1_x1_x1] = off;
    }
    {
      itk::Offset<3> off = {{1,0,-1}};
      offsetMap[DIR_x1_x0_n1] = off;
    }
    {
      itk::Offset<3> off = {{1,1,-1}};
      offsetMap[DIR_x1_x1_n1] = off;
    }
    {
      itk::Offset<3> off = {{0,1,-1}};
      offsetMap[DIR_x0_x1_n1] = off;
    }
    {
      itk::Offset<3> off = {{-1,1,-1}};
      offsetMap[DIR_n1_x1_n1] = off;
    }
    {
      itk::Offset<3> off = {{0,0,1}};
      offsetMap[DIR_x0_x0_x1] = off;
    }
    return offsetMap;
  }

  int                                   m_DirectionFlags;
  std::map<OffsetDirection, OffsetType> m_offsetMap;
  unsigned int                          m_levels;

  void DirectionFlags(int flags)
  {
    DirectionFlags(static_cast<OffsetDirection>(flags));
  }

  void DirectionFlags(OffsetDirection flags)
  {
    m_DirectionFlags = flags;
  }

  void SetLevels(unsigned int lvl)
  {
    m_levels = lvl;
  }

  NeighborhoodCooccurenceMatrix()
  {
    m_offsetMap = CreateStdOffsets();
    m_levels = 5;
    m_DirectionFlags =
        DIR_x1_x0_x0; /*| DIR_x1_x1_x0 | DIR_x0_x1_x0 |
        DIR_n1_x1_x0 | DIR_x1_x0_x1 | DIR_x1_x1_x1 |
        DIR_x0_x1_x1 | DIR_n1_x1_x1 | DIR_x1_x0_n1 |
        DIR_x1_x1_n1 | DIR_x0_x1_n1 | DIR_n1_x1_n1 |
        DIR_x0_x0_x1;*/

    std::cout << "NeighborhoodCooccurenceMatrix" << std::endl;
  }

  inline OutputVectorType operator()(const TNeighborhoodType & it) const
  {
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::min();
    for (unsigned int i = 0; i < it.Size(); ++i)
    {
      double value = it.GetPixel(i);
      max = (value > max) ? value : max;
      min = (value < min) ? value : min;
    }
    if ( min >= max)
    {
      OutputVectorType nullRes; nullRes.fill(0);
      return nullRes;
    }

    SizeType size;
    MeasurementVectorType minBorder;
    MeasurementVectorType maxBorder;
    OffsetType g1, g2;

    size.SetSize(2); // 2D Historgram
    size.Fill(m_levels); // 5 bins each dim

    minBorder.SetSize(2); // min range value
    minBorder.Fill(min);

    maxBorder.SetSize(2); // max range value
    maxBorder.Fill(max);

    MeasurementVectorType cooccur;
    cooccur.SetSize(2);

    OutputVectorType output_vector;
    output_vector.fill(0);

    double div_num_dirs = 0;

    //std::bitset<14> x(m_DirectionFlags);
    //std::cout << "Direction flag " << x;

    for(typename std::map<OffsetDirection,OffsetType>::const_iterator dir_it = m_offsetMap.begin(),
        end = m_offsetMap.end(); dir_it != end; dir_it ++){

      if(! (dir_it->first & m_DirectionFlags))
        continue;

      div_num_dirs++;

      HistogramType::Pointer histogram = HistogramType::New();
      histogram->SetMeasurementVectorSize(2);
      histogram->Initialize(size, minBorder, maxBorder);

      for (unsigned int i = 0; i < it.Size(); ++i)
      {
        // grayvalue pair (g1,g2)
        // g1 ~ g2 with ~ as relation (e.g. a offset calculation)
        g1 = it.GetOffset(i);
        g2 = g1 + dir_it->second;

        cooccur[0] = it.GetPixel(i);
        cooccur[1] = it.GetImagePointer()->GetPixel(it.GetIndex(i)+dir_it->second);

        histogram->IncreaseFrequencyOfMeasurement(cooccur, 1);

        std::swap(cooccur[0],cooccur[1]);
        histogram->IncreaseFrequencyOfMeasurement(cooccur, 1);
      }

      //To do Normalize GLCM N_g Number of levels

      HistoToFeatureFilter::Pointer filter = HistoToFeatureFilter::New();
      filter->SetInput(histogram);
      filter->Update();

      output_vector[ENERGY] += filter->GetEnergy();
      output_vector[ENTROPY] += filter->GetEntropy();
      output_vector[CORRELATION] += filter->GetCorrelation();
      output_vector[INERTIA] += filter->GetInertia();
      output_vector[CLUSTERSHADE] += filter->GetClusterShade();
      output_vector[CLUSTERPROMINENCE] += filter->GetClusterProminence();
      output_vector[HARALICKCORRELATION] += filter->GetHaralickCorrelation();
      output_vector[INVERSEDIFFERENCEMOMENT] += filter->GetInverseDifferenceMoment();
    }

    //std::cout << "Number of directions " << div_num_dirs << std::endl;
//    output_vector /= div_num_dirs;
    return output_vector;
  }
};

}// end namespace functor

} // end namespace itk

#endif // itkNeighborhoodFunctors_h
