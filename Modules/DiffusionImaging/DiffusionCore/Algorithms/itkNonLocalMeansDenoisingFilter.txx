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
  : m_SearchRadius(5),
    m_ComparisonRadius(1),
    m_UseJointInformation(true),
    m_UseRicianAdaption(false),
    m_Variance(536.87)
{
  this->SetNumberOfRequiredInputs( 2 );
}

template< class TPixelType >
void
NonLocalMeansDenoisingFilter< TPixelType >
::BeforeThreadedGenerateData()
{
//  typename OutputImageType::Pointer outputImage =
//          static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
//  typename OutputImageType::PixelType px;
//  px.SetSize(1);
//  px.SetElement(0,0);
//  outputImage->FillBuffer(px);

//  typename InputImageType::Pointer inImage = static_cast< InputImageType* >(this->ProcessObject::GetInput(0));
//  typename MaskImageType::Pointer mask = static_cast< MaskImageType* >(this->ProcessObject::GetInput(1));
//  int size = inImage->GetVectorLength();
//  m_Deviations.SetSize(size);
//  typename ImageExtractorType::Pointer extractor = ImageExtractorType::New();
//  extractor->SetInput(inImage);

//  // calculate max value of mask, for correct inversion
//  typename StatisticsFilterType::Pointer statisticsFilter = StatisticsFilterType::New();
//  statisticsFilter->SetInput(mask);
//  statisticsFilter->Update();

//  // invert mask, to mask the backround
//  typename InvertImageFilterType::Pointer inverter = InvertImageFilterType::New();
//  inverter->SetInput(mask);
//  inverter->SetMaximum(statisticsFilter->GetMaximum());
//  inverter->Update();

//  // make sure inverted mask has same origin is the brainmask
//  typename ChangeInformationType::Pointer changeMaskFilter = ChangeInformationType::New();
//  changeMaskFilter->ChangeOriginOn();
//  changeMaskFilter->SetInput(inverter->GetOutput());
//  changeMaskFilter->SetOutputOrigin(mask->GetOrigin());
//  changeMaskFilter->Update();
//  typename MaskImageType::Pointer invertedMask = changeMaskFilter->GetOutput();
//  typename MaskImageType::PointType imageOrigin = inImage->GetOrigin();
//  typename MaskImageType::PointType maskOrigin = invertedMask->GetOrigin();
//  long offset[3];

//  typedef itk::ContinuousIndex<double, 3> ContinousIndexType;
//  ContinousIndexType maskOriginContinousIndex, imageOriginContinousIndex;

//  inImage->TransformPhysicalPointToContinuousIndex(maskOrigin, maskOriginContinousIndex);
//  inImage->TransformPhysicalPointToContinuousIndex(imageOrigin, imageOriginContinousIndex);

//  // make sure there is no misalignment between mask and image
//  for ( unsigned int i = 0; i < 3; ++i )
//  {
//    double misalignment = maskOriginContinousIndex[i] - floor( maskOriginContinousIndex[i] + 0.5 );
//    if ( fabs( misalignment ) > mitk::eps )
//    {
//        itkExceptionMacro( << "Pixels/voxels of mask and image are not sufficiently aligned! (Misalignment: " << misalignment << ")" );
//    }

//    double indexCoordDistance = maskOriginContinousIndex[i] - imageOriginContinousIndex[i];
//    offset[i] = (int) indexCoordDistance + inImage->GetBufferedRegion().GetIndex()[i];
//  }

//  // calculate for each channel the stddev
//  for ( int i = 0; i < size; ++i)
//  {
//    /// extract channel i of the input
//    extractor->SetIndex(i);
//    extractor->Update();

//    // adapt mask to the image
//    typename ChangeInformationType::Pointer adaptMaskFilter;
//    adaptMaskFilter = ChangeInformationType::New();
//    adaptMaskFilter->ChangeOriginOn();
//    adaptMaskFilter->ChangeRegionOn();
//    adaptMaskFilter->SetInput( invertedMask );
//    adaptMaskFilter->SetOutputOrigin( extractor->GetOutput()->GetOrigin() /*image->GetOrigin()*/ );
//    adaptMaskFilter->SetOutputOffset( offset );
//    adaptMaskFilter->Update();

//    // extract backround as the ROI
//    typename MaskImageType::Pointer adaptedMaskImage = adaptMaskFilter->GetOutput();
//    typename ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
//    extractImageFilter->SetInput( extractor->GetOutput() );
//    extractImageFilter->SetExtractionRegion( adaptedMaskImage->GetBufferedRegion() );
//    extractImageFilter->Update();

//    // calculate statistics of ROI
//    typename MaskImageType::Pointer adaptedImage = extractImageFilter->GetOutput();
//    typename LabelStatisticsFilterType::Pointer labelStatisticsFilter = LabelStatisticsFilterType::New();
//    labelStatisticsFilter->SetInput(adaptedImage);
//    labelStatisticsFilter->SetLabelInput(adaptedMaskImage);
//    labelStatisticsFilter->UseHistogramsOff();
//    labelStatisticsFilter->GetOutput()->SetRequestedRegion( adaptedMaskImage->GetLargestPossibleRegion() );
//    labelStatisticsFilter->Update();

//    // save the stddev of each channel
//    m_Deviations.SetElement(i, labelStatisticsFilter->GetSigma(1));
//  }

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
  git.GoToBegin();

  m_Variance = 536.87;

  // iterate over complete image region
  while( !git.IsAtEnd() )
  {


    typename OutputImageType::PixelType outpix;
    outpix.SetSize (inputImagePointer->GetVectorLength());

    if(!m_UseJointInformation)
    {
      for (int i = 0; i < (int)inputImagePointer->GetVectorLength(); ++i)
      {
        double Z = 0;
        double sumj = 0;
        double w = 0;
        std::vector<double> wj;
        std::vector<double> p;
        typename InputIteratorType::IndexType index;
        index = git.GetIndex();

        for (int x = index.GetElement(0) - m_SearchRadius; x <= index.GetElement(0) + m_SearchRadius; ++x)
        {
          for (int y = index.GetElement(1) - m_SearchRadius; y <= index.GetElement(1) + m_SearchRadius; ++y)
          {
            for (int z = index.GetElement(2) - m_SearchRadius; z <= index.GetElement(2) + m_SearchRadius; ++z)
            {
              typename InputIteratorType::IndexType indexV;
              indexV.SetElement(0, x);
              indexV.SetElement(1, y);
              indexV.SetElement(2, z);
              if (inputImagePointer->GetLargestPossibleRegion().IsInside(indexV))
              {
                TPixelType pixelJ = inputImagePointer->GetPixel(indexV)[i];
                double sumk = 0;
                double size = 0;
                for (int xi = index.GetElement(0) - m_ComparisonRadius, xj = x - m_ComparisonRadius; xi <= index.GetElement(0) + m_ComparisonRadius; ++xi, ++xj)
                {
                  for (int yi = index.GetElement(1) - m_ComparisonRadius, yj = y - m_ComparisonRadius; yi <= index.GetElement(1) + m_ComparisonRadius; ++yi, ++yj)
                  {
                    for (int zi = index.GetElement(2) - m_ComparisonRadius, zj = z - m_ComparisonRadius; zi <= index.GetElement(2) + m_ComparisonRadius; ++zi, ++zj)
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
                        int diff = inputImagePointer->GetPixel(indexI)[i] - inputImagePointer->GetPixel(indexJ)[i];
                        sumk += (double)(diff*diff);
                        ++size;
                      }
                    }
                  }
                }
                // weight all neighborhoods
                w = std::exp( - (sumk / size) / m_Variance);
                wj.push_back(w);
                if (m_UseRicianAdaption)
                {
                  p.push_back((double)(pixelJ*pixelJ));
                }
                else
                {
                  p.push_back((double)(pixelJ));
                }
                Z += w;
              }
            }
          }
        }
        for (unsigned int n = 0; n < wj.size(); ++n)
        {
          sumj += (wj[n]/Z) * p[n];
        }
        double df;
        if (m_UseRicianAdaption)
        {
          df = sumj - (2 * m_Variance);
        }
        else
        {
          df = sumj;
        }
        if (df < 0)
        {
          df = 0;
        }

        TPixelType outval = std::floor(std::sqrt(df) + 0.5);
        outpix.SetElement(i, outval);
      }
    }

    else
    {
      double Z = 0;
      itk::VariableLengthVector<double> sumj;
      sumj.SetSize(inputImagePointer->GetVectorLength());
      sumj.Fill(0.0);
      double w = 0;
      std::vector<double> wj;
      std::vector<itk::VariableLengthVector <double> > p;
      typename InputIteratorType::IndexType index;
      index = git.GetIndex();

      for (int x = index.GetElement(0) - m_SearchRadius; x <= index.GetElement(0) + m_SearchRadius; ++x)
      {
        for (int y = index.GetElement(1) - m_SearchRadius; y <= index.GetElement(1) + m_SearchRadius; ++y)
        {
          for (int z = index.GetElement(2) - m_SearchRadius; z <= index.GetElement(2) + m_SearchRadius; ++z)
          {
            typename InputIteratorType::IndexType indexV;
            indexV.SetElement(0, x);
            indexV.SetElement(1, y);
            indexV.SetElement(2, z);
            if (inputImagePointer->GetLargestPossibleRegion().IsInside(indexV))
            {
              typename InputImageType::PixelType pixelJ = inputImagePointer->GetPixel(indexV);
              double sumk = 0;
              double size = 0;
              for (int xi = index.GetElement(0) - m_ComparisonRadius, xj = x - m_ComparisonRadius; xi <= index.GetElement(0) + m_ComparisonRadius; ++xi, ++xj)
              {
                for (int yi = index.GetElement(1) - m_ComparisonRadius, yj = y - m_ComparisonRadius; yi <= index.GetElement(1) + m_ComparisonRadius; ++yi, ++yj)
                {
                  for (int zi = index.GetElement(2) - m_ComparisonRadius, zj = z - m_ComparisonRadius; zi <= index.GetElement(2) + m_ComparisonRadius; ++zi, ++zj)
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
                      typename InputImageType::PixelType diff = inputImagePointer->GetPixel(indexI) - inputImagePointer->GetPixel(indexJ);
                      sumk += (double)(diff.GetSquaredNorm());
                      ++size;
                    }
                  }
                }
              }
              // weight all neighborhoods
              w = std::exp( - (sumk / size) / m_Variance);
              wj.push_back(w);
              if (m_UseRicianAdaption)
              {
                itk::VariableLengthVector <double> m;
                m.SetSize(inputImagePointer->GetVectorLength());
                for (unsigned int i = 0; i < inputImagePointer->GetVectorLength(); ++i)
                {
                  double sp = (double)(pixelJ.GetElement(i) * pixelJ.GetElement(i));
                  m.SetElement(i,sp);
                }
                p.push_back(m);
              }
              else
              {
                p.push_back(pixelJ);
              }
              Z += w;
            }
          }
        }
      }
      for (unsigned int n = 0; n < wj.size(); ++n)
      {
        sumj += (wj[n]/Z) * p[n];
      }
      typename InputImageType::PixelType df;
      if (m_UseRicianAdaption)
      {
        df = sumj - (2 * m_Variance);
      }
      else
      {
        df = sumj;
      }
      for (unsigned int i = 0; i < inputImagePointer->GetVectorLength(); ++i)
      {
        double a = df.GetElement(i);
        if (a < 0)
        {
          a = 0;
        }
        TPixelType outval = std::floor(std::sqrt(a) + 0.5);
        outpix.SetElement(i, outval);
      }
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
