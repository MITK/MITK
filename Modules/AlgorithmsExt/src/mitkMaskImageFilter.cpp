/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMaskImageFilter.h"
#include "mitkImageTimeSelector.h"
#include "mitkProperties.h"
#include "mitkTimeHelper.h"

#include "mitkImageAccessByItk.h"
#include "mitkImageToItk.h"

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIteratorWithIndex.h"

#include <limits>

mitk::MaskImageFilter::MaskImageFilter() : m_Mask(nullptr)
{
  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfRequiredInputs(2);
  m_InputTimeSelector = mitk::ImageTimeSelector::New();
  m_MaskTimeSelector = mitk::ImageTimeSelector::New();
  m_OutputTimeSelector = mitk::ImageTimeSelector::New();
  m_OverrideOutsideValue = false;
  m_OutsideValue = 0;
}

mitk::MaskImageFilter::~MaskImageFilter()
{
}

void mitk::MaskImageFilter::SetMask(const mitk::Image *mask)
{
  // Process object is not const-correct so the const_cast is required here
  m_Mask = const_cast<mitk::Image *>(mask);
  this->ProcessObject::SetNthInput(1, m_Mask);
}

const mitk::Image *mitk::MaskImageFilter::GetMask() const
{
  return m_Mask;
}

void mitk::MaskImageFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Image *output = this->GetOutput();
  mitk::Image *input = this->GetInput();
  mitk::Image *mask = m_Mask;
  if ((output->IsInitialized() == false) || (mask == nullptr) || (mask->GetTimeGeometry()->CountTimeSteps() == 0))
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

  itkDebugMacro(<< "GenerateOutputInformation()");

  output->Initialize(input->GetPixelType(), *input->GetTimeGeometry());

  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::MaskImageFilter::InternalComputeMask(itk::Image<TPixel, VImageDimension> *inputItkImage)
{
  // dirty quick fix, duplicating code so both unsigned char and unsigned short are supported
  // this should be changed once unsigned char segmentations can be converted to unsigned short
  mitk::PixelType pixelType =
    m_MaskTimeSelector->GetOutput()->GetImageDescriptor()->GetChannelDescriptor().GetPixelType();
  if (pixelType.GetComponentType() == itk::ImageIOBase::UCHAR)
  {
    typedef itk::Image<TPixel, VImageDimension> ItkInputImageType;
    typedef itk::Image<unsigned char, VImageDimension> ItkMaskImageType;
    typedef itk::Image<TPixel, VImageDimension> ItkOutputImageType;

    typedef itk::ImageRegionConstIterator<ItkInputImageType> ItkInputImageIteratorType;
    typedef itk::ImageRegionConstIterator<ItkMaskImageType> ItkMaskImageIteratorType;
    typedef itk::ImageRegionIteratorWithIndex<ItkOutputImageType> ItkOutputImageIteratorType;

    typename mitk::ImageToItk<ItkMaskImageType>::Pointer maskimagetoitk = mitk::ImageToItk<ItkMaskImageType>::New();
    maskimagetoitk->SetInput(m_MaskTimeSelector->GetOutput());
    maskimagetoitk->Update();
    typename ItkMaskImageType::Pointer maskItkImage = maskimagetoitk->GetOutput();

    typename mitk::ImageToItk<ItkOutputImageType>::Pointer outputimagetoitk =
      mitk::ImageToItk<ItkOutputImageType>::New();
    outputimagetoitk->SetInput(m_OutputTimeSelector->GetOutput());
    outputimagetoitk->Update();
    typename ItkOutputImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();

    // create the iterators
    typename ItkInputImageType::RegionType inputRegionOfInterest = inputItkImage->GetLargestPossibleRegion();
    ItkInputImageIteratorType inputIt(inputItkImage, inputRegionOfInterest);
    ItkMaskImageIteratorType maskIt(maskItkImage, inputRegionOfInterest);
    ItkOutputImageIteratorType outputIt(outputItkImage, inputRegionOfInterest);

    // typename ItkOutputImageType::PixelType outsideValue = itk::NumericTraits<typename
    // ItkOutputImageType::PixelType>::min();
    if (!m_OverrideOutsideValue)
      m_OutsideValue = itk::NumericTraits<typename ItkOutputImageType::PixelType>::min();

    m_MinValue = std::numeric_limits<mitk::ScalarType>::max();
    m_MaxValue = std::numeric_limits<mitk::ScalarType>::min();

    for (inputIt.GoToBegin(), maskIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd() && !maskIt.IsAtEnd();
         ++inputIt, ++maskIt, ++outputIt)
    {
      if (maskIt.Get() > itk::NumericTraits<typename ItkMaskImageType::PixelType>::Zero)
      {
        outputIt.Set(inputIt.Get());
        m_MinValue = vnl_math_min((float)inputIt.Get(), (float)m_MinValue);
        m_MaxValue = vnl_math_max((float)inputIt.Get(), (float)m_MaxValue);
      }
      else
      {
        outputIt.Set(m_OutsideValue);
      }
    }
  }
  else
  {
    {
      typedef itk::Image<TPixel, VImageDimension> ItkInputImageType;
      typedef itk::Image<unsigned short, VImageDimension> ItkMaskImageType;
      typedef itk::Image<TPixel, VImageDimension> ItkOutputImageType;

      typedef itk::ImageRegionConstIterator<ItkInputImageType> ItkInputImageIteratorType;
      typedef itk::ImageRegionConstIterator<ItkMaskImageType> ItkMaskImageIteratorType;
      typedef itk::ImageRegionIteratorWithIndex<ItkOutputImageType> ItkOutputImageIteratorType;

      typename mitk::ImageToItk<ItkMaskImageType>::Pointer maskimagetoitk = mitk::ImageToItk<ItkMaskImageType>::New();
      maskimagetoitk->SetInput(m_MaskTimeSelector->GetOutput());
      maskimagetoitk->Update();
      typename ItkMaskImageType::Pointer maskItkImage = maskimagetoitk->GetOutput();

      typename mitk::ImageToItk<ItkOutputImageType>::Pointer outputimagetoitk =
        mitk::ImageToItk<ItkOutputImageType>::New();
      outputimagetoitk->SetInput(m_OutputTimeSelector->GetOutput());
      outputimagetoitk->Update();
      typename ItkOutputImageType::Pointer outputItkImage = outputimagetoitk->GetOutput();

      // create the iterators
      typename ItkInputImageType::RegionType inputRegionOfInterest = inputItkImage->GetLargestPossibleRegion();
      ItkInputImageIteratorType inputIt(inputItkImage, inputRegionOfInterest);
      ItkMaskImageIteratorType maskIt(maskItkImage, inputRegionOfInterest);
      ItkOutputImageIteratorType outputIt(outputItkImage, inputRegionOfInterest);

      // typename ItkOutputImageType::PixelType outsideValue = itk::NumericTraits<typename
      // ItkOutputImageType::PixelType>::min();
      if (!m_OverrideOutsideValue)
        m_OutsideValue = itk::NumericTraits<typename ItkOutputImageType::PixelType>::min();

      m_MinValue = std::numeric_limits<mitk::ScalarType>::max();
      m_MaxValue = std::numeric_limits<mitk::ScalarType>::min();

      for (inputIt.GoToBegin(), maskIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd() && !maskIt.IsAtEnd();
           ++inputIt, ++maskIt, ++outputIt)
      {
        if (maskIt.Get() > itk::NumericTraits<typename ItkMaskImageType::PixelType>::Zero)
        {
          outputIt.Set(inputIt.Get());
          m_MinValue = vnl_math_min((float)inputIt.Get(), (float)m_MinValue);
          m_MaxValue = vnl_math_max((float)inputIt.Get(), (float)m_MaxValue);
        }
        else
        {
          outputIt.Set(m_OutsideValue);
        }
      }
    }
  }
}

void mitk::MaskImageFilter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer mask = m_Mask;
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized() == false) || (mask.IsNull()) || (mask->GetTimeGeometry()->CountTimeSteps() == 0))
    return;

  m_InputTimeSelector->SetInput(input);
  m_MaskTimeSelector->SetInput(mask);
  m_OutputTimeSelector->SetInput(this->GetOutput());

  mitk::Image::RegionType outputRegion = output->GetRequestedRegion();
  const mitk::TimeGeometry *outputTimeGeometry = output->GetTimeGeometry();
  const mitk::TimeGeometry *inputTimeGeometry = input->GetTimeGeometry();
  const mitk::TimeGeometry *maskTimeGeometry = mask->GetTimeGeometry();
  ScalarType timeInMS;

  int timestep = 0;
  int tstart = outputRegion.GetIndex(3);
  int tmax = tstart + outputRegion.GetSize(3);

  int t;
  for (t = tstart; t < tmax; ++t)
  {
    timeInMS = outputTimeGeometry->TimeStepToTimePoint(t);

    timestep = inputTimeGeometry->TimePointToTimeStep(timeInMS);

    m_InputTimeSelector->SetTimeNr(timestep);
    m_InputTimeSelector->UpdateLargestPossibleRegion();
    m_OutputTimeSelector->SetTimeNr(t);
    m_OutputTimeSelector->UpdateLargestPossibleRegion();

    timestep = maskTimeGeometry->TimePointToTimeStep(timeInMS);
    m_MaskTimeSelector->SetTimeNr(timestep);
    m_MaskTimeSelector->UpdateLargestPossibleRegion();

    AccessByItk(m_InputTimeSelector->GetOutput(), InternalComputeMask);
  }

  m_TimeOfHeaderInitialization.Modified();
}
