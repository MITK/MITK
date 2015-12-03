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

#include "mitkMRNormLinearStatisticBasedFilter.h"

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

mitk::MRNormLinearStatisticBasedFilter::MRNormLinearStatisticBasedFilter() :
  m_CenterMode(MRNormLinearStatisticBasedFilter::MEDIAN)
{
  this->SetNumberOfIndexedInputs(3);
  this->SetNumberOfRequiredInputs(3);
}

mitk::MRNormLinearStatisticBasedFilter::~MRNormLinearStatisticBasedFilter()
{
}

void mitk::MRNormLinearStatisticBasedFilter::SetMask( const mitk::Image* mask )
{
  // Process object is not const-correct so the const_cast is required here
  Image* nonconstMask = const_cast< mitk::Image * >( mask );
  this->SetNthInput(1, nonconstMask );
}

const mitk::Image* mitk::MRNormLinearStatisticBasedFilter::GetMask() const
{
  return this->GetInput(1);
}

void mitk::MRNormLinearStatisticBasedFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Image* input = const_cast< mitk::Image * > ( this->GetInput() );

  input->SetRequestedRegionToLargestPossibleRegion();
}

void mitk::MRNormLinearStatisticBasedFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  output->Initialize(input->GetPixelType(), *input->GetTimeGeometry());
  output->SetPropertyList(input->GetPropertyList()->Clone());
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::MRNormLinearStatisticBasedFilter::InternalComputeMask(itk::Image<TPixel, VImageDimension>* itkImage)
{
  // Define all necessary Types
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<int, VImageDimension> MaskType;
  typedef itk::LabelStatisticsImageFilter<ImageType, MaskType> FilterType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;

  typename MaskType::Pointer itkMask0 = MaskType::New();
  mitk::CastToItkImage(this->GetMask(), itkMask0);

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
  double stddev = labelStatisticsImageFilter->GetSigma(1);
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

  double value0=0;
  switch (m_CenterMode)
  {
  case MRNormLinearStatisticBasedFilter::MEAN:
    value0=mean0; break;
  case MRNormLinearStatisticBasedFilter::MEDIAN:
    value0=median0; break;
  case MRNormLinearStatisticBasedFilter::MODE:
    value0=modulo0; break;
  }

  double offset = value0;
  double scaling = stddev;
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

void mitk::MRNormLinearStatisticBasedFilter::GenerateData()
{
  AccessByItk(GetInput(0),InternalComputeMask);
}