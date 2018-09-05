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

#ifndef __itkEvaluateDirectionImagesFilter_cpp
#define __itkEvaluateDirectionImagesFilter_cpp

#include "itkEvaluateDirectionImagesFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkImageDuplicator.h>
#include <boost/progress.hpp>

namespace itk {

template< class PixelType >
ComparePeakImagesFilter< PixelType >
::ComparePeakImagesFilter():
  m_IgnoreMissingRefDirections(false),
  m_IgnoreMissingTestDirections(false),
  m_Eps(0.0001)
{
  this->SetNumberOfIndexedOutputs(2);
}

template< class PixelType >
void ComparePeakImagesFilter< PixelType >::GenerateData()
{
  if (m_TestImage.IsNull() || m_ReferenceImage.IsNull())
    mitkThrow() << "Test or reference image not set!";
//  if (m_TestImage->GetLargestPossibleRegion().GetSize()!=m_ReferenceImage->GetLargestPossibleRegion().GetSize())
//    mitkThrow() << "Test and reference image need to have same sizes!";

  Vector<float, 4> spacing4 = m_TestImage->GetSpacing();
  Point<float, 4> origin4 = m_TestImage->GetOrigin();
  Matrix<double, 4, 4> direction4 = m_TestImage->GetDirection();
  ImageRegion<4> imageRegion4 = m_TestImage->GetLargestPossibleRegion();

  Vector<double, 3> spacing3;
  Point<float, 3> origin3;
  Matrix<double, 3, 3> direction3;
  ImageRegion<3> imageRegion3;

  spacing3[0] = spacing4[0]; spacing3[1] = spacing4[1]; spacing3[2] = spacing4[2];
  origin3[0] = origin4[0]; origin3[1] = origin4[1]; origin3[2] = origin4[2];
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
      direction3[r][c] = direction4[r][c];
  imageRegion3.SetSize(0, imageRegion4.GetSize()[0]);
  imageRegion3.SetSize(1, imageRegion4.GetSize()[1]);
  imageRegion3.SetSize(2, imageRegion4.GetSize()[2]);

  // angular error image
  typename OutputImageType::Pointer outputImage = OutputImageType::New();
  outputImage->SetOrigin( origin3 );
  outputImage->SetRegions( imageRegion3 );
  outputImage->SetSpacing( spacing3 );
  outputImage->SetDirection( direction3 );
  outputImage->Allocate();
  outputImage->FillBuffer(0.0);
  this->SetNthOutput(0, outputImage);

  // length error image
  outputImage = OutputImageType::New();
  outputImage->SetOrigin( origin3 );
  outputImage->SetRegions( imageRegion3 );
  outputImage->SetSpacing( spacing3 );
  outputImage->SetDirection( direction3 );
  outputImage->Allocate();
  outputImage->FillBuffer(0.0);
  this->SetNthOutput(1, outputImage);

  if (m_MaskImage.IsNull())
  {
    m_MaskImage = UCharImageType::New();
    m_MaskImage->SetOrigin( origin3 );
    m_MaskImage->SetRegions( imageRegion3 );
    m_MaskImage->SetSpacing( spacing3 );
    m_MaskImage->SetDirection( direction3 );
    m_MaskImage->Allocate();
    m_MaskImage->FillBuffer(1);
  }

  m_MeanAngularError = 0.0;
  m_MedianAngularError = 0;
  m_MaxAngularError = 0.0;
  m_MinAngularError = itk::NumericTraits<float>::max();
  m_VarAngularError = 0.0;
  m_AngularErrorVector.clear();

  m_MeanLengthError = 0.0;
  m_MedianLengthError = 0;
  m_MaxLengthError = 0.0;
  m_MinLengthError = itk::NumericTraits<float>::max();
  m_VarLengthError = 0.0;
  m_LengthErrorVector.clear();

  outputImage = static_cast< OutputImageType* >(this->ProcessObject::GetOutput(0));
  typename OutputImageType::Pointer outputImage2 = static_cast< OutputImageType* >(this->ProcessObject::GetOutput(1));

  ImageRegionIterator< OutputImageType > oit(outputImage, outputImage->GetLargestPossibleRegion());
  ImageRegionIterator< OutputImageType > oit2(outputImage2, outputImage2->GetLargestPossibleRegion());
  ImageRegionIterator< UCharImageType > mit(m_MaskImage, m_MaskImage->GetLargestPossibleRegion());


  boost::progress_display disp(outputImage->GetLargestPossibleRegion().GetSize()[0]*outputImage->GetLargestPossibleRegion().GetSize()[1]*outputImage->GetLargestPossibleRegion().GetSize()[2]);
  while( !oit.IsAtEnd() )
  {
    ++disp;
    if( mit.Get()!=1 )
    {
      ++oit;
      ++oit2;
      ++mit;
      continue;
    }
    typename OutputImageType::IndexType index = oit.GetIndex();
    itk::Index<4> idx4;
    idx4[0] = index[0];
    idx4[1] = index[1];
    idx4[2] = index[2];

    std::vector< PixelType > ref_magnitudes;
    std::vector< PeakType > ref_peaks;
    for (unsigned int i=0; i<m_ReferenceImage->GetLargestPossibleRegion().GetSize()[3]/3; i++)
    {
      PeakType ref_peak;
      idx4[3] = i*3;
      ref_peak[0] = m_ReferenceImage->GetPixel(idx4);
      idx4[3] = i*3 + 1;
      ref_peak[1] = m_ReferenceImage->GetPixel(idx4);
      idx4[3] = i*3 + 2;
      ref_peak[2] = m_ReferenceImage->GetPixel(idx4);

      PixelType ref_mag = ref_peak.magnitude();
      if (ref_mag > m_Eps )
      {
        ref_peak.normalize();
        ref_peaks.push_back(ref_peak);
        ref_magnitudes.push_back(ref_mag);
      }
    }

    std::vector< PixelType > test_magnitudes;
    std::vector< PeakType > test_peaks;
    for (unsigned int j=0; j<m_TestImage->GetLargestPossibleRegion().GetSize()[3]/3; j++)
    {
      PeakType test_peak;
      idx4[3] = j*3;
      test_peak[0] = m_TestImage->GetPixel(idx4);
      idx4[3] = j*3 + 1;
      test_peak[1] = m_TestImage->GetPixel(idx4);
      idx4[3] = j*3 + 2;
      test_peak[2] = m_TestImage->GetPixel(idx4);

      PixelType test_mag = test_peak.magnitude();
      if (test_mag > m_Eps )
      {
        test_peak.normalize();
        test_peaks.push_back(test_peak);
        test_magnitudes.push_back(test_mag);
      }
    }

    if (test_peaks.empty() && ref_peaks.empty())
    {
      ++oit;
      ++oit2;
      ++mit;
      continue;
    }
    else if (test_peaks.empty())
    {
      if (!m_IgnoreMissingTestDirections)
      {
        PixelType length_sum = 0;
        for (auto l : ref_magnitudes)
        {
          length_sum += l;
          m_AngularErrorVector.push_back(90);
          m_LengthErrorVector.push_back(l);
          m_MeanAngularError += m_AngularErrorVector.back();
          m_MeanLengthError += m_LengthErrorVector.back();
        }
        oit2.Set(length_sum/ref_magnitudes.size());
        oit.Set(90);
      }
    }
    else if (ref_peaks.empty())
    {
      if (!m_IgnoreMissingRefDirections)
      {
        PixelType length_sum = 0;
        for (auto l : test_magnitudes)
        {
          length_sum += l;
          m_AngularErrorVector.push_back(90);
          m_LengthErrorVector.push_back(length_sum);
          m_MeanAngularError += m_AngularErrorVector.back();
          m_MeanLengthError += m_LengthErrorVector.back();
        }
        oit2.Set(length_sum/test_magnitudes.size());
        oit.Set(90);
      }
    }
    else
    {
      PixelType error_a = 0;
      PixelType error_l = 0;
      for (unsigned int i=0; i<ref_peaks.size(); ++i)
      {
        PixelType max_a = 0;
        PixelType max_l = 0;
        for (unsigned int j=0; j<test_peaks.size(); ++j)
        {
          auto a = fabs(dot_product(test_peaks[j], ref_peaks[i]));
          if (a>1.0)
            a = 1.0;
          if (a>max_a)
          {
            max_a = a;
            max_l = fabs(ref_magnitudes[i]-test_magnitudes[j]);
          }
        }
        m_LengthErrorVector.push_back( max_l );
        m_AngularErrorVector.push_back( acos(max_a)*180.0/itk::Math::pi );
        m_MeanAngularError += m_AngularErrorVector.back();
        m_MeanLengthError += m_LengthErrorVector.back();
        error_a += m_AngularErrorVector.back();
        error_l += m_LengthErrorVector.back();
      }

      for (unsigned int i=0; i<test_peaks.size(); ++i)
      {
        PixelType max_a = 0;
        PixelType max_l = 0;
        for (unsigned int j=0; j<ref_peaks.size(); ++j)
        {
          auto a = fabs(dot_product(test_peaks[i], ref_peaks[j]));
          if (a>1.0)
            a = 1.0;
          if (a>max_a)
          {
            max_a = a;
            max_l = fabs(ref_magnitudes[j]-test_magnitudes[i]);
          }
        }
        m_LengthErrorVector.push_back( max_l );
        m_AngularErrorVector.push_back( acos(max_a)*180.0/itk::Math::pi );
        m_MeanAngularError += m_AngularErrorVector.back();
        m_MeanLengthError += m_LengthErrorVector.back();
        error_a += m_AngularErrorVector.back();
        error_l += m_LengthErrorVector.back();
      }

      error_a /= (test_peaks.size() + ref_peaks.size());
      error_l /= (test_peaks.size() + ref_peaks.size());

      oit2.Set(error_l);
      oit.Set(error_a);
    }

    ++oit;
    ++oit2;
    ++mit;
  }

  std::sort( m_AngularErrorVector.begin(), m_AngularErrorVector.end() );
  if (!m_AngularErrorVector.empty())
    m_MeanAngularError /= m_AngularErrorVector.size();      // mean


  for (unsigned int i=0; i<m_AngularErrorVector.size(); i++)
  {
    float temp = m_AngularErrorVector.at(i) - m_MeanAngularError;
    m_VarAngularError += temp*temp;

    if ( m_AngularErrorVector.at(i)>m_MaxAngularError )
      m_MaxAngularError = m_AngularErrorVector.at(i);
    if ( m_AngularErrorVector.at(i)<m_MinAngularError )
      m_MinAngularError = m_AngularErrorVector.at(i);
  }
  if (m_AngularErrorVector.size()>1)
  {
    m_VarAngularError /= (m_AngularErrorVector.size()-1);   // variance

    // median
    if (m_AngularErrorVector.size()%2 == 0)
      m_MedianAngularError = 0.5*( m_AngularErrorVector.at( m_AngularErrorVector.size()/2 ) +  m_AngularErrorVector.at( m_AngularErrorVector.size()/2+1 ) );
    else
      m_MedianAngularError = m_AngularErrorVector.at( (m_AngularErrorVector.size()+1)/2 ) ;
  }

  std::sort( m_LengthErrorVector.begin(), m_LengthErrorVector.end() );
  m_MeanLengthError /= m_LengthErrorVector.size();      // mean
  for (unsigned int i=0; i<m_LengthErrorVector.size(); i++)
  {
    float temp = m_LengthErrorVector.at(i) - m_MeanLengthError;
    m_VarLengthError += temp*temp;

    if ( m_LengthErrorVector.at(i)>m_MaxLengthError )
      m_MaxLengthError = m_LengthErrorVector.at(i);
    if ( m_LengthErrorVector.at(i)<m_MinLengthError )
      m_MinLengthError = m_LengthErrorVector.at(i);
  }
  if (m_LengthErrorVector.size()>1)
  {
    m_VarLengthError /= (m_LengthErrorVector.size()-1);   // variance

    // median
    if (m_LengthErrorVector.size()%2 == 0)
      m_MedianLengthError = 0.5*( m_LengthErrorVector.at( m_LengthErrorVector.size()/2 ) +  m_LengthErrorVector.at( m_LengthErrorVector.size()/2+1 ) );
    else
      m_MedianLengthError = m_LengthErrorVector.at( (m_LengthErrorVector.size()+1)/2 ) ;
  }
}

}

#endif // __itkEvaluateDirectionImagesFilter_cpp
