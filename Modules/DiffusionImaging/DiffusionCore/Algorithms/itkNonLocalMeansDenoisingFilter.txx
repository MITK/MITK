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

#ifndef __itkNonLocalMeansDenoisingFilter_txx
#define __itkNonLocalMeansDenoisingFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "itkImageRegionIterator.h"
#include "itkNeighborhoodIterator.h"
#include <itkImageRegionIteratorWithIndex.h>
#include <vector>

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

  // calculate max value of mask, for correct inversion
  typename StatisticsFilterType::Pointer statisticsFilter = StatisticsFilterType::New();
  statisticsFilter->SetInput(mask);
  statisticsFilter->Update();

  // invert mask, to mask the backround
  typename InvertImageFilterType::Pointer inverter = InvertImageFilterType::New();
  inverter->SetInput(mask);
  inverter->SetMaximum(statisticsFilter->GetMaximum());
  inverter->Update();

  // make sure inverted mask has same origin is the brainmask
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

  // make sure there is no misalignment between mask and image
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

  // calculate for each channel the stddev
  for ( int i = 0; i < size; ++i)
  {
    /// extract channel i of the input
    extractor->SetIndex(i);
    extractor->Update();

    // adapt mask to the image
    typename ChangeInformationType::Pointer adaptMaskFilter;
    adaptMaskFilter = ChangeInformationType::New();
    adaptMaskFilter->ChangeOriginOn();
    adaptMaskFilter->ChangeRegionOn();
    adaptMaskFilter->SetInput( invertedMask );
    adaptMaskFilter->SetOutputOrigin( extractor->GetOutput()->GetOrigin() /*image->GetOrigin()*/ );
    adaptMaskFilter->SetOutputOffset( offset );
    adaptMaskFilter->Update();

    // extract backround as the ROI
    typename MaskImageType::Pointer adaptedMaskImage = adaptMaskFilter->GetOutput();
    typename ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
    extractImageFilter->SetInput( extractor->GetOutput() );
    extractImageFilter->SetExtractionRegion( adaptedMaskImage->GetBufferedRegion() );
    extractImageFilter->Update();

    // calculate statistics of ROI
    typename MaskImageType::Pointer adaptedImage = extractImageFilter->GetOutput();
    typename LabelStatisticsFilterType::Pointer labelStatisticsFilter = LabelStatisticsFilterType::New();
    labelStatisticsFilter->SetInput(adaptedImage);
    labelStatisticsFilter->SetLabelInput(adaptedMaskImage);
    labelStatisticsFilter->UseHistogramsOff();
    labelStatisticsFilter->GetOutput()->SetRequestedRegion( adaptedMaskImage->GetLargestPossibleRegion() );
    labelStatisticsFilter->Update();

    // save the stddev of each channel
    m_Deviations.SetElement(i, labelStatisticsFilter->GetSigma(1));
  }

  m_CurrentVoxelCount = 0;
}

template< class TPixelType >
void
NonLocalMeansDenoisingFilter< TPixelType >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, ThreadIdType )
{
  // initialize iterators
  typename OutputImageType::Pointer outputImage =
          static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

  ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
  oit.GoToBegin();

  typedef ImageRegionIteratorWithIndex <InputImageType> InputIteratorType;
  typename InputImageType::Pointer inputImagePointer = NULL;
  inputImagePointer = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

  InputIteratorType git(inputImagePointer, outputRegionForThread );
  InputIteratorType njit(inputImagePointer, outputRegionForThread );
  InputIteratorType niit(inputImagePointer, outputRegionForThread );
  InputIteratorType hit(inputImagePointer, outputRegionForThread);
  git.GoToBegin();

  // iterate over complete image region
  while( !git.IsAtEnd() )
  {


    typename OutputImageType::PixelType outpix;
    outpix.SetSize (inputImagePointer->GetVectorLength());

    for (int i = 0; i < (int)inputImagePointer->GetVectorLength(); ++i)
    {
      double Z = 0;
      double sumj = 0;
      double w = 0;
      double variance = m_Deviations.GetElement(i) * m_Deviations.GetElement(i);
      std::vector<double> wj;
      std::vector<double> p;

      for (int x = git.GetIndex().GetElement(0) - m_SearchRadius; x <= git.GetIndex().GetElement(0) + m_SearchRadius; ++x)
      {
        for (int y = git.GetIndex().GetElement(1) - m_SearchRadius; y <= git.GetIndex().GetElement(1) + m_SearchRadius; ++y)
        {
          for (int z = git.GetIndex().GetElement(2) - m_SearchRadius; z <= git.GetIndex().GetElement(2) + m_SearchRadius; ++z)
          {
            typename InputIteratorType::IndexType idx;
            idx.SetElement(0, x);
            idx.SetElement(1, y);
            idx.SetElement(2, z);
            if (inputImagePointer->GetLargestPossibleRegion().IsInside(idx))
            {
              hit.SetIndex(idx);
              TPixelType pixelJ = hit.Get()[i];
              double sumk = 0;
              double size = 0;
              for (int xi = git.GetIndex().GetElement(0) - m_ComparisonRadius, xj = hit.GetIndex().GetElement(0) - m_ComparisonRadius; xi <= git.GetIndex().GetElement(0) + m_ComparisonRadius; ++xi, ++xj)
              {
                for (int yi = git.GetIndex().GetElement(1) - m_ComparisonRadius, yj = hit.GetIndex().GetElement(1) - m_ComparisonRadius; yi <= git.GetIndex().GetElement(1) + m_ComparisonRadius; ++yi, ++yj)
                {
                  for (int zi = git.GetIndex().GetElement(2) - m_ComparisonRadius, zj = hit.GetIndex().GetElement(2) - m_ComparisonRadius; zi <= git.GetIndex().GetElement(2) + m_ComparisonRadius; ++zi, ++zj)
                  {
                    typename InputIteratorType::IndexType indexI, indexJ;
                    indexI.SetElement(0, xi);
                    indexI.SetElement(1, yi);
                    indexI.SetElement(2, zi);
                    indexJ.SetElement(0, xj);
                    indexJ.SetElement(1, yj);
                    indexJ.SetElement(2, zj);
                    // Compare neighborhoods ni & nj
                    if (inputImagePointer->GetLargestPossibleRegion().IsInside(indexI) && inputImagePointer->GetLargestPossibleRegion().IsInside(indexJ))
                    {
                      niit.SetIndex(indexI);
                      njit.SetIndex(indexJ);
                      if (m_UseJointInformation)
                      {
                        // if filter should use joint information. A 4d Neighborhood including surrounding channels is used.
                        for (int d = i - m_ChannelRadius; d <= i + m_ChannelRadius; ++d)
                        {
                          if (d >= 0 && d < (int)inputImagePointer->GetVectorLength())
                          {
                            int diff = niit.Get()[d] - njit.Get()[d];
                            sumk += (double)(diff*diff);
                            ++size;
                          }
                        }
                      }
                      else
                      {
                        int diff = niit.Get()[i] - njit.Get()[i];
                        sumk += (double)(diff*diff);
                        ++size;
                      }
                    }
                  }
                }
              }
              // weight all neighborhoods
              w = std::exp( - (sumk / size) / variance);
              wj.push_back(w);
              p.push_back((double)(pixelJ*pixelJ));
              Z += w;
            }
          }
        }
      }
      for (unsigned int n = 0; n < wj.size(); ++n)
      {
        sumj += (wj[n]/Z) * p[n];
      }
      double df = sumj - (2 * variance);
      if (df < 0)
      {
        df = 0;
      }

      TPixelType outval = std::floor(std::sqrt(df) + 0.5);
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
