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

#include "mitkMRNormTwoRegionBasedFilter.h"

#include "mitkImageToItk.h"
#include "mitkImageAccessByItk.h"

#include "itkImageRegionIterator.h"
// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

mitk::MRNormTwoRegionsBasedFilter::MRNormTwoRegionsBasedFilter() :
  m_Area1(MRNormTwoRegionsBasedFilter::MEDIAN),
  m_Area2(MRNormTwoRegionsBasedFilter::MEDIAN)
{
  this->SetNumberOfIndexedInputs(3);
  this->SetNumberOfRequiredInputs(1);
}

mitk::MRNormTwoRegionsBasedFilter::~MRNormTwoRegionsBasedFilter()
{
}

void mitk::MRNormTwoRegionsBasedFilter::SetMask1( const mitk::Image* mask )
{
  // Process object is not const-correct so the const_cast is required here
  auto* nonconstMask = const_cast< mitk::Image * >( mask );
  this->SetNthInput(1, nonconstMask );
}

void mitk::MRNormTwoRegionsBasedFilter::SetMask2( const mitk::Image* mask )
{
  // Process object is not const-correct so the const_cast is required here
  auto* nonconstMask = const_cast< mitk::Image * >( mask );
  this->SetNthInput(2, nonconstMask );
}

const mitk::Image* mitk::MRNormTwoRegionsBasedFilter::GetMask1() const
{
  return this->GetInput(1);
}

const mitk::Image* mitk::MRNormTwoRegionsBasedFilter::GetMask2() const
{
  return this->GetInput(2);
}

void mitk::MRNormTwoRegionsBasedFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Image* input = this->GetInput();

  input->SetRequestedRegionToLargestPossibleRegion();
}

void mitk::MRNormTwoRegionsBasedFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  output->Initialize(input->GetPixelType(), *input->GetTimeGeometry());
  output->SetPropertyList(input->GetPropertyList()->Clone());
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::MRNormTwoRegionsBasedFilter::InternalComputeMask(itk::Image<TPixel, VImageDimension>* itkImage)
{
  // Define all necessary Types
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<int, VImageDimension> MaskType;
  typedef itk::LabelStatisticsImageFilter<ImageType, MaskType> FilterType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;

  typename MaskType::Pointer itkMask0 = MaskType::New();
  typename MaskType::Pointer itkMask1 = MaskType::New();
  mitk::CastToItkImage(this->GetMask1(), itkMask0);
  mitk::CastToItkImage(this->GetMask2(), itkMask1);

  typename ImageType::Pointer outImage = ImageType::New();
  mitk::CastToItkImage(this->GetOutput(0), outImage);

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  typename FilterType::Pointer labelStatisticsImageFilter = FilterType::New();
  labelStatisticsImageFilter->SetUseHistograms(true);
  labelStatisticsImageFilter->SetHistogramParameters(256, minMaxComputer->GetMinimum(),minMaxComputer->GetMaximum());
  labelStatisticsImageFilter->SetInput( itkImage );

  labelStatisticsImageFilter->SetLabelInput(itkMask0);
  labelStatisticsImageFilter->Update();
  double median0 = labelStatisticsImageFilter->GetMedian(1);
  double mean0 = labelStatisticsImageFilter->GetMean(1);
  double modulo0=0;
  {
    auto histo = labelStatisticsImageFilter->GetHistogram(1);
    double maxFrequency=0;
    for (auto hIter=histo->Begin();hIter!=histo->End();++hIter)
    {
      if (maxFrequency < hIter.GetFrequency())
      {
        maxFrequency = hIter.GetFrequency();
        modulo0 = (histo->GetBinMin(0,hIter.GetInstanceIdentifier()) + histo->GetBinMax(0,hIter.GetInstanceIdentifier())) / 2.0;
      }
    }
  }
  labelStatisticsImageFilter->SetLabelInput(itkMask1);
  labelStatisticsImageFilter->Update();
  double median1 = labelStatisticsImageFilter->GetMedian(1);
  double mean1 = labelStatisticsImageFilter->GetMean(1);
  double modulo1 = 0;
  {
    auto histo = labelStatisticsImageFilter->GetHistogram(1);
    double maxFrequency=0;
    for (auto hIter=histo->Begin();hIter!=histo->End();++hIter)
    {
      if (maxFrequency < hIter.GetFrequency())
      {
        maxFrequency = hIter.GetFrequency();
        modulo1 = (histo->GetBinMin(0,hIter.GetInstanceIdentifier()) + histo->GetBinMax(0,hIter.GetInstanceIdentifier())) / 2.0;
      }
    }
  }

  double value0=0;
  double value1=0;
  switch (m_Area1)
  {
  case MRNormTwoRegionsBasedFilter::MEAN:
    value0=mean0; break;
  case MRNormTwoRegionsBasedFilter::MEDIAN:
    value0=median0; break;
  case MRNormTwoRegionsBasedFilter::MODE:
    value0=modulo0; break;
  }
  switch (m_Area2)
  {
  case MRNormTwoRegionsBasedFilter::MEAN:
    value1=mean1; break;
  case MRNormTwoRegionsBasedFilter::MEDIAN:
    value1=median1; break;
  case MRNormTwoRegionsBasedFilter::MODE:
    value1=modulo1; break;
  }

  double offset = std::min(value0, value1);
  double scaling = std::max(value0, value1) - offset;
  if (scaling < 0.0001)
    return;

  itk::ImageRegionIterator<ImageType> inIter(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<ImageType> outIter(outImage, outImage->GetLargestPossibleRegion());
  while (! inIter.IsAtEnd())
  {
    TPixel value = inIter.Value();
    outIter.Set((value - offset) / scaling);
    ++inIter;
    ++outIter;
  }
}

void mitk::MRNormTwoRegionsBasedFilter::GenerateData()
{
  AccessByItk(GetInput(0),InternalComputeMask);
}