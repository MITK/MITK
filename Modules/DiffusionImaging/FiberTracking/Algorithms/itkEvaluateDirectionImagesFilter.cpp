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
  m_IgnoreMissingDirections(false),
  m_IgnoreEmptyVoxels(false),
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

    float maxAngularError = 1.0;

    // get number of valid directions (length > 0)
    unsigned int numRefPeaks = 0;
    unsigned int numTestPeaks = 0;
    std::vector< vnl_vector_fixed< PixelType, 3 > > testPeaks;
    std::vector< vnl_vector_fixed< PixelType, 3 > > refPeaks;
    std::vector< PixelType > testPeakMagnitudes;
    std::vector< PixelType > refPeakMagnitudes;

    for (unsigned int i=0; i<m_ReferenceImage->GetLargestPossibleRegion().GetSize()[3]/3; i++)
    {
      vnl_vector_fixed< PixelType, 3 > peak;
      idx4[3] = i*3;
      peak[0] = m_ReferenceImage->GetPixel(idx4);
      idx4[3] = i*3 + 1;
      peak[1] = m_ReferenceImage->GetPixel(idx4);
      idx4[3] = i*3 + 2;
      peak[2] = m_ReferenceImage->GetPixel(idx4);

      PixelType mag = peak.magnitude();
      refPeakMagnitudes.push_back(mag);
      if (mag > m_Eps )
      {
        peak.normalize();
        refPeaks.push_back(peak);
        numRefPeaks++;
      }
    }
    for (unsigned int i=0; i<m_TestImage->GetLargestPossibleRegion().GetSize()[3]/3; i++)
    {
      vnl_vector_fixed< PixelType, 3 > peak;
      idx4[3] = i*3;
      peak[0] = m_TestImage->GetPixel(idx4);
      idx4[3] = i*3 + 1;
      peak[1] = m_TestImage->GetPixel(idx4);
      idx4[3] = i*3 + 2;
      peak[2] = m_TestImage->GetPixel(idx4);

      PixelType mag = peak.magnitude();
      testPeakMagnitudes.push_back(mag);
      if (mag > m_Eps )
      {
        peak.normalize();
        testPeaks.push_back(peak);
        numTestPeaks++;
      }
    }


    // matrix containing the angular error between the directions
    unsigned int maxNumPeaks = std::max(numTestPeaks, numRefPeaks);
    vnl_matrix< float > diffM; diffM.set_size(maxNumPeaks, maxNumPeaks);
    diffM.fill(0.0);
    diffM.fill(10); // initialize with invalid error value

    if (m_IgnoreEmptyVoxels && (numRefPeaks==0 || numTestPeaks==0) )
    {
      ++oit;
      ++oit2;
      ++mit;
      continue;
    }

    // i: index of reference direction
    // j: index of test direction
    for (unsigned int i=0; i<maxNumPeaks; i++)     // for each reference direction
    {
      bool missingDir = false;
      vnl_vector_fixed< PixelType, 3 > refDir;

      if (i<numRefPeaks)  // normalize if not null
        refDir = refPeaks.at(i);
      else if (m_IgnoreMissingDirections)
        continue;
      else
        missingDir = true;

      for (unsigned int j=0; j<maxNumPeaks; j++)     // and each test direction
      {
        vnl_vector_fixed< PixelType, 3 > testDir;
        if (j<numTestPeaks)  // normalize if not null
          testDir = testPeaks.at(j);
        else if (m_IgnoreMissingDirections || missingDir)
          continue;
        else
          missingDir = true;

        // found missing direction
        if (missingDir)
        {
          diffM[i][j] = -1;
          continue;
        }

        // calculate angle between directions
        diffM[i][j] = fabs(dot_product(refDir, testDir));

        if (diffM[i][j] < maxAngularError)
          maxAngularError = diffM[i][j];

        if (diffM[i][j]>1.0)
          diffM[i][j] = 1.0;
      }
    }

    float angularError = 0.0;
    float lengthError = 0.0;
    int counter = 0;
    vnl_matrix< float > diffM_copy = diffM;
    for (unsigned int k=0; k<maxNumPeaks; k++)
    {
      float error = -1;
      int a,b; a=-1; b=-1;
      bool missingDir = false;

      // i: index of reference direction
      // j: index of test direction
      // find smalles error between two directions (large value -> small error)
      for (unsigned int i=0; i<maxNumPeaks; i++)
        for (unsigned int j=0; j<maxNumPeaks; j++)
        {
          if (diffM[i][j]>error && diffM[i][j]<2) // found valid error entry
          {
            error = diffM[i][j];
            a = (int)i;
            b = (int)j;
            missingDir = false;
          }
          else if (diffM[i][j]<0 && error<0)    // found missing direction
          {
            a = (int)i;
            b = (int)j;
            missingDir = true;
          }
        }

      if (a<0 || b<0 || (m_IgnoreMissingDirections && missingDir))
        continue; // no more directions found

      if (a>=(int)numRefPeaks && b>=(int)numTestPeaks)
      {
        MITK_INFO << a << " " << numRefPeaks;
        MITK_INFO << b << " " << numTestPeaks;
        MITK_INFO << "ERROR: missing test and reference direction. should not be possible. check code.";
        continue;
      }

      // remove processed directions from error matrix
      diffM.set_row(a, 10.0);
      diffM.set_column(b, 10.0);

      if (a>=(int)numRefPeaks) // missing reference direction (find next closest)
      {
        for (unsigned int i=0; i<numRefPeaks; i++)
          if (diffM_copy[i][b]>error)
          {
            error = diffM_copy[i][b];
            a = i;
          }
      }
      else if (b>=(int)numTestPeaks) // missing test direction (find next closest)
      {
        for (unsigned int i=0; i<numTestPeaks; i++)
          if (diffM_copy[a][i]>error)
          {
            error = diffM_copy[a][i];
            b = i;
          }
      }

      float refLength = 0;
      float testLength = 1;

      if (a>=(int)numRefPeaks || b>=(int)numTestPeaks || error<0)
        error = 0;
      else
      {
        refLength = refPeakMagnitudes.at(a);
        testLength = testPeakMagnitudes.at(b);
      }

      m_LengthErrorVector.push_back( fabs(refLength-testLength) );
      m_AngularErrorVector.push_back( acos(error)*180.0/itk::Math::pi );

      m_MeanAngularError += m_AngularErrorVector.back();
      m_MeanLengthError += m_LengthErrorVector.back();

      angularError += m_AngularErrorVector.back();
      lengthError += m_LengthErrorVector.back();
      counter++;
    }

    if (counter>0)
    {
      lengthError /= counter;
      angularError /= counter;
    }
    oit2.Set(lengthError);
    oit.Set(angularError);

    ++oit;
    ++oit2;
    ++mit;
  }

  std::sort( m_AngularErrorVector.begin(), m_AngularErrorVector.end() );
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
