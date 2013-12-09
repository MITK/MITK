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


#include "mitkMaskImageFilter.h"
#include "mitkImageTimeSelector.h"
#include "mitkTimeHelper.h"
#include "mitkProperties.h"
#include "mitkImageCast.h"
#include "mitkImageToItk.h"
#include "mitkImageAccessByItk.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIteratorWithIndex.h"

#include <limits>

mitk::MaskImageFilter::MaskImageFilter() : m_Mask(NULL)
{
  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfRequiredInputs(2);
  m_InputTimeSelector  = mitk::ImageTimeSelector::New();
  m_MaskTimeSelector   = mitk::ImageTimeSelector::New();
  m_OutputTimeSelector = mitk::ImageTimeSelector::New();
  m_OverrideOutsideValue = false;
  m_OverrideInsideValue = false;
  m_OutsideValue = 0.0;
  m_InsideValue = 1.0;
}

mitk::MaskImageFilter::~MaskImageFilter()
{

}

void mitk::MaskImageFilter::SetMask( const mitk::Image* mask )
{
  // Process object is not const-correct so the const_cast is required here
  m_Mask = const_cast< mitk::Image * >( mask );
  this->ProcessObject::SetNthInput(1, m_Mask );
}

const mitk::Image* mitk::MaskImageFilter::GetMask() const
{
  return m_Mask;
}

void mitk::MaskImageFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Image* output = this->GetOutput();
  mitk::Image* input = const_cast< mitk::Image * > ( this->GetInput() );
  mitk::Image* mask  = m_Mask ;
  if((output->IsInitialized()==false) || (mask == NULL) || (mask->GetTimeGeometry()->CountTimeSteps() == 0))
    return;

  input->SetRequestedRegionToLargestPossibleRegion();
  mask->SetRequestedRegionToLargestPossibleRegion();

  GenerateTimeInInputRegion(output, input);
  GenerateTimeInInputRegion(output, mask);
}

void mitk::MaskImageFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized()) && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  itkDebugMacro(<<"GenerateOutputInformation()");

  output->Initialize(input->GetPixelType(), *input->GetTimeGeometry());

  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}

template <typename TPixel1, unsigned int VDimension1, typename TPixel2, unsigned int VDimension2>
void mitk::MaskImageFilter::InternalComputeMask( itk::Image<TPixel1, VDimension1>* referenceImage,
                                                 const itk::Image<TPixel2, VDimension2>* maskImage )
{
  typedef typename itk::Image< TPixel1, VDimension1> ReferenceImageType;
  typedef typename itk::Image< TPixel2, VDimension2> MaskImageType;

  typedef itk::ImageRegionConstIterator< ReferenceImageType > ReferenceImageIteratorType;
  typedef itk::ImageRegionConstIterator< MaskImageType >      MaskImageIteratorType;
  typedef itk::ImageRegionIteratorWithIndex< ReferenceImageType > OutputImageIteratorType;

  mitk::Image::Pointer output = this->GetOutput();

  typename ReferenceImageType::Pointer outputImage;
  mitk::CastToItkImage<ReferenceImageType>(output, outputImage);

  // create the iterators
  typename ReferenceImageType::RegionType inputRegionOfInterest = referenceImage->GetLargestPossibleRegion();
  ReferenceImageIteratorType  inputIt( referenceImage, inputRegionOfInterest );
  MaskImageIteratorType  maskIt ( maskImage, inputRegionOfInterest );
  OutputImageIteratorType outputIt( outputImage, inputRegionOfInterest );

  //typename ItkOutputImageType::PixelType outsideValue = itk::NumericTraits<typename ItkOutputImageType::PixelType>::min();
//  if ( !m_OverrideOutsideValue )
//    m_OutsideValue = itk::NumericTraits<typename ItkOutputImageType::PixelType>::min();

  m_MinValue = std::numeric_limits<mitk::ScalarType>::max();
  m_MaxValue = std::numeric_limits<mitk::ScalarType>::min();

  for ( inputIt.GoToBegin(), maskIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd() && !maskIt.IsAtEnd(); ++inputIt, ++maskIt, ++outputIt)
  {
    if ( maskIt.Get() > itk::NumericTraits<typename MaskImageType::PixelType>::Zero )
    {
      if (m_OverrideInsideValue)
        outputIt.Set( m_InsideValue );
      else
        outputIt.Set( inputIt.Get() );
      m_MinValue = vnl_math_min((float)inputIt.Get(), (float)m_MinValue);
      m_MaxValue = vnl_math_max((float)inputIt.Get(), (float)m_MaxValue);
    }
    else if (m_OverrideOutsideValue)
    {
      outputIt.Set( m_OutsideValue );
    }
    else
    {
      outputIt.Set( inputIt.Get() );
    }
  }
}

void mitk::MaskImageFilter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer mask  = m_Mask;
  mitk::Image::Pointer output = this->GetOutput();

  if((output->IsInitialized()==false) || (mask.IsNull()) || (mask->GetTimeGeometry()->CountTimeSteps() == 0))
    return;

  m_InputTimeSelector->SetInput(input);
  m_MaskTimeSelector->SetInput(mask);
  m_OutputTimeSelector->SetInput(this->GetOutput());

  mitk::Image::RegionType outputRegion = output->GetRequestedRegion();
  const mitk::TimeGeometry *outputTimeGeometry = output->GetTimeGeometry();
  const mitk::TimeGeometry *inputTimeGeometry = input->GetTimeGeometry();
  const mitk::TimeGeometry *maskTimeGeometry = mask->GetTimeGeometry();
  ScalarType timeInMS;

  int timestep=0;
  int tstart=outputRegion.GetIndex(3);
  int tmax=tstart+outputRegion.GetSize(3);

  int t;
  for(t=tstart;t<tmax;++t)
  {
    timeInMS = outputTimeGeometry->TimeStepToTimePoint( t );

    timestep = inputTimeGeometry->TimePointToTimeStep( timeInMS );

    m_InputTimeSelector->SetTimeNr(timestep);
    m_InputTimeSelector->UpdateLargestPossibleRegion();
    m_OutputTimeSelector->SetTimeNr(t);
    m_OutputTimeSelector->UpdateLargestPossibleRegion();

    timestep = maskTimeGeometry->TimePointToTimeStep( timeInMS );
    m_MaskTimeSelector->SetTimeNr(timestep);
    m_MaskTimeSelector->UpdateLargestPossibleRegion();

    AccessTwoImagesFixedDimensionByItk(m_InputTimeSelector->GetOutput(), m_MaskTimeSelector->GetOutput(), InternalComputeMask, 3);
  }

  m_TimeOfHeaderInitialization.Modified();
}
