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

#ifndef __itkNonLocalMeansDenoisingFilter_cpp
#define __itkNonLocalMeansDenoisingFilter_cpp

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "itkImageRegionIterator.h"
#include "itkNeighborhoodIterator.h"
#include <itkImageRegionIteratorWithIndex.h>

namespace itk {


template< class TPixelType >
NonLocalMeansDenoisingFilter< TPixelType >
::NonLocalMeansDenoisingFilter()
{
  this->SetNumberOfRequiredInputs( 2 );
}

template< class TPixelType >
void
NonLocalMeansDenoisingFilter< TPixelType >
::BeforeThreadedGenerateData()
{
  typename OutputImageType::Pointer outputImage =
          static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
  typename OutputImageType::PixelType px;
  px.SetSize(1);
  px.SetElement(0,0);
  outputImage->FillBuffer(px);

  typename InputImageType::Pointer inImage = static_cast< InputImageType* >(this->ProcessObject::GetInput(0));
  typename MaskImageType::Pointer mask = static_cast< MaskImageType* >(this->ProcessObject::GetInput(1));
  int size = inImage->GetVectorLength();
  m_Deviations.SetSize(size);
  typename ImageExtractorType::Pointer extractor = ImageExtractorType::New();
  extractor->SetInput(inImage);

  // Generate backround mask out of brain mask to calculate stdev of noise
  typename StatisticsFilterType::Pointer statisticsFilter = StatisticsFilterType::New();
  statisticsFilter->SetInput(mask);
  statisticsFilter->Update();
  typename InvertImageFilterType::Pointer inverter = InvertImageFilterType::New();
  inverter->SetInput(mask);
  inverter->SetMaximum(statisticsFilter->GetMaximum());
  typename ChangeInformationType::Pointer changeMaskFilter = ChangeInformationType::New();
  changeMaskFilter->ChangeOriginOn();
  changeMaskFilter->SetInput(inverter->GetOutput());
  changeMaskFilter->SetOutputOrigin(mask->GetOrigin());
  changeMaskFilter->Update();
  typename MaskImageType::Pointer invertedMask = changeMaskFilter->GetOutput();
  typename MaskImageType::PointType imageOrigin = inImage->GetOrigin();
  typename MaskImageType::PointType maskOrigin = invertedMask->GetOrigin();
  long offset[3];

  typedef itk::ContinuousIndex<double, 3> ContinousIndexType;
  ContinousIndexType maskOriginContinousIndex, imageOriginContinousIndex;

  inImage->TransformPhysicalPointToContinuousIndex(maskOrigin, maskOriginContinousIndex);
  inImage->TransformPhysicalPointToContinuousIndex(imageOrigin, imageOriginContinousIndex);

  for ( unsigned int i = 0; i < 3; ++i )
  {
    double misalignment = maskOriginContinousIndex[i] - floor( maskOriginContinousIndex[i] + 0.5 );
    if ( fabs( misalignment ) > mitk::eps )
    {
        itkExceptionMacro( << "Pixels/voxels of mask and image are not sufficiently aligned! (Misalignment: " << misalignment << ")" );
    }

    double indexCoordDistance = maskOriginContinousIndex[i] - imageOriginContinousIndex[i];
    offset[i] = (int) indexCoordDistance + inImage->GetBufferedRegion().GetIndex()[i];
  }

  // Claculate stdev of noise for each channel of the vectorimage
  for ( int i = 0; i < size; ++i)
  {
    extractor->SetIndex(i);
    extractor->Update();
    typename ChangeInformationType::Pointer adaptMaskFilter;
    adaptMaskFilter = ChangeInformationType::New();
    adaptMaskFilter->ChangeOriginOn();
    adaptMaskFilter->ChangeRegionOn();
    adaptMaskFilter->SetInput( invertedMask );
    adaptMaskFilter->SetOutputOrigin( extractor->GetOutput()->GetOrigin() );
    adaptMaskFilter->SetOutputOffset( offset );
    adaptMaskFilter->Update();
    typename MaskImageType::Pointer adaptedMaskImage = adaptMaskFilter->GetOutput();
    typename ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
    extractImageFilter->SetInput( extractor->GetOutput() );
    extractImageFilter->SetExtractionRegion( adaptedMaskImage->GetBufferedRegion() );
    extractImageFilter->Update();
    typename MaskImageType::Pointer adaptedImage = extractImageFilter->GetOutput();
    typename LabelStatisticsFilterType::Pointer labelStatisticsFilter = LabelStatisticsFilterType::New();
    labelStatisticsFilter->SetInput(adaptedImage);
    labelStatisticsFilter->SetLabelInput(adaptedMaskImage);
    labelStatisticsFilter->UseHistogramsOff();
    labelStatisticsFilter->GetOutput()->SetRequestedRegion( adaptedMaskImage->GetLargestPossibleRegion() );
    labelStatisticsFilter->Update();
    m_Deviations.SetElement(i, labelStatisticsFilter->GetSigma(0));
  }

  m_CurrentVoxelCount = 0;
}

template< class TPixelType >
void
NonLocalMeansDenoisingFilter< TPixelType >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
  // Initialize Iterators
  typename OutputImageType::Pointer outputImage =
          static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

  ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
  oit.GoToBegin();

  typedef ImageRegionIteratorWithIndex <InputImageType> InputIteratorType;
  typename InputImageType::Pointer inputImagePointer = NULL;
  inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

  InputIteratorType git(/*m_V_Radius,*/ inputImagePointer, outputRegionForThread );
  InputIteratorType njit(/*m_N_Radius,*/ inputImagePointer, outputRegionForThread );
  InputIteratorType niit(/*m_N_Radius,*/ inputImagePointer, outputRegionForThread );
  InputIteratorType hit(inputImagePointer, outputRegionForThread);
  git.GoToBegin();

  // Iterate over complete image region
  while( !git.IsAtEnd() )
  {
//    double sz = (double)niit.Size();
    typename OutputImageType::PixelType outpix;
    outpix.SetSize (inputImagePointer->GetVectorLength());

    // Count amount of same voxels in neighborhood V around xi, to determine normalization constant Z
    for (unsigned int i = 0; i < inputImagePointer->GetVectorLength(); ++i)
    {
      TPixelType pixelI = git.Get()[i];
      double sumj = 0;
      double wj = 0;
      short Z = 0;
      double size = pow((m_N_Radius[0]*2+1), 3);
      if (m_UseJointInformation)
      {
//        for (unsigned int n = 0; n < inputImagePointer->GetVectorLength(); ++n)
//        {
//          for (unsigned int j = 0; j < git.Size(); ++j)
//          {
//            bool isInBounds = false;
//            TPixelType xj = git.GetPixel(j, isInBounds)[n];
//            if (xi == xj && isInBounds)
//            {
//              ++Z;
//            }
//          }
//        }
      }
      else
      {
        for (int x = (int)git.GetIndex().GetElement(0) - (int)m_V_Radius[0]; x <= (int)git.GetIndex().GetElement(0) + (int)m_V_Radius[0]; ++x)
        {
          for (int y = (int)git.GetIndex().GetElement(1) - (int)m_V_Radius[1]; y <= (int)git.GetIndex().GetElement(1) + (int)m_V_Radius[1]; ++y)
          {
            for (int z = (int)git.GetIndex().GetElement(2) - (int)m_V_Radius[2]; z <= (int)git.GetIndex().GetElement(2) + (int)m_V_Radius[2]; ++z)
            {
              typename InputIteratorType::IndexType idx;
              idx.SetElement(0, x);
              idx.SetElement(1, y);
              idx.SetElement(2, z);
              if (inputImagePointer->GetLargestPossibleRegion().IsInside(idx))
              {
                hit.SetIndex(idx);
                TPixelType pixelJ = hit.Get()[i];
                if (pixelI == pixelJ)
                {
                  ++Z;
                }
              }
            }
          }
        }
      }
      if (m_UseJointInformation)
      {
//        for (unsigned int n = 0; n < inputImagePointer->GetVectorLength(); ++n)
//        {
//          for (unsigned int j = 0; j < git.Size(); ++j)
//          {
//            bool isInBounds = false;
//            TPixelType xj = git.GetPixel(j, isInBounds)[n];
//            if (xi == xj && isInBounds)
//            {
//              ++Z;
//            }
//          }
//        }
      }
      else
      {
        for (int x = (int)git.GetIndex().GetElement(0) - (int)m_V_Radius[0]; x <= (int)git.GetIndex().GetElement(0) + (int)m_V_Radius[0]; ++x)
        {
          for (int y = (int)git.GetIndex().GetElement(1) - (int)m_V_Radius[1]; y <= (int)git.GetIndex().GetElement(1) + (int)m_V_Radius[1]; ++y)
          {
            for (int z = (int)git.GetIndex().GetElement(2) - (int)m_V_Radius[2]; z <= (int)git.GetIndex().GetElement(2) + (int)m_V_Radius[2]; ++z)
            {
              typename InputIteratorType::IndexType idx;
              idx.SetElement(0, x);
              idx.SetElement(1, y);
              idx.SetElement(2, z);
              if (inputImagePointer->GetLargestPossibleRegion().IsInside(idx))
              {
                hit.SetIndex(idx);
                TPixelType pixelJ = hit.Get()[i];
                if (pixelI == pixelJ)
                {
                  double sumk = 0;
                  for (int xi = git.GetIndex().GetElement(0) - (int) m_N_Radius[0], xj = hit.GetIndex().GetElement(0) - (int) m_N_Radius[0]; xi <= git.GetIndex().GetElement(0) + (int) m_N_Radius[0]; ++xi, ++xj)
                  {
                    for (int yi = git.GetIndex().GetElement(1) - (int) m_N_Radius[1], yj = hit.GetIndex().GetElement(1) - (int) m_N_Radius[1]; yi <= git.GetIndex().GetElement(1) + (int) m_N_Radius[1]; ++yi, ++yj)
                    {
                      for (int zi = git.GetIndex().GetElement(2) - (int) m_N_Radius[2], zj = hit.GetIndex().GetElement(2) - (int) m_N_Radius[2]; zi <= git.GetIndex().GetElement(2) + (int) m_N_Radius[2]; ++zi, ++zj)
                      {
                        typename InputIteratorType::IndexType indexI, indexJ;
                        indexI.SetElement(0, xi);
                        indexI.SetElement(1, yi);
                        indexI.SetElement(2, zi);
                        indexJ.SetElement(0, xj);
                        indexJ.SetElement(1, yj);
                        indexJ.SetElement(2, zj);
                        if (inputImagePointer->GetLargestPossibleRegion().IsInside(indexI) && inputImagePointer->GetLargestPossibleRegion().IsInside(indexJ))
                        {
                          niit.SetIndex(indexI);
                          njit.SetIndex(indexJ);
                          int diff = niit.Get()[i] - njit.Get()[i];
                          sumk += std::pow((double)diff, 2);
                        }
                      }
                    }
                  }
                  wj = std::exp( - (std::sqrt((sumk / size)) / m_Deviations.GetElement(i)) / Z);
                  sumj += (wj * std::pow((double) (pixelJ), 2)) - (2 * std::pow(m_Deviations.GetElement(i), 2));
                }
              }
            }
          }
        }
      }
      if (sumj < 0)
      {
        sumj = 0;
      }

      TPixelType outval = std::floor(std::sqrt(sumj) + 0.5);
      outpix.SetElement(i, outval);
    }
    oit.Set(outpix);
    ++oit;

    ++m_CurrentVoxelCount;
    ++git;
  }

  MITK_INFO << "One Thread finished calculation";
}

template< class TPixelType >
void NonLocalMeansDenoisingFilter< TPixelType >::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}

template< class TPixelType >
void NonLocalMeansDenoisingFilter< TPixelType >::SetNRadius(unsigned int n)
{
  for (int i = 0; i < InputImageType::ImageDimension; ++i)
  {
    m_N_Radius [i] = n;
  }
}

template< class TPixelType >
void NonLocalMeansDenoisingFilter< TPixelType >::SetVRadius(unsigned int v)
{
  for (int i = 0; i < InputImageType::ImageDimension; ++i)
  {
    m_V_Radius [i] = v;
  }
}

template< class TPixelType >
void NonLocalMeansDenoisingFilter< TPixelType >::SetInputImage(const InputImageType* image)
{
  this->SetNthInput(0, const_cast< InputImageType* >(image));
}

template< class TPixelType >
void NonLocalMeansDenoisingFilter< TPixelType >::SetInputMask(const MaskImageType* mask)
{
  this->SetNthInput(1, const_cast< MaskImageType* >(mask));
}

}

#endif // __itkNonLocalMeansDenoisingFilter_txx
