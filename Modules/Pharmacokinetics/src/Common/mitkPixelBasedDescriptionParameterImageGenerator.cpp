/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "itkCommand.h"
#include "itkMultiOutputNaryFunctorImageFilter.h"

#include "mitkPixelBasedDescriptionParameterImageGenerator.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkSimpleFunctorPolicy.h"
#include "mitkModelBase.h"

void
  mitk::PixelBasedDescriptionParameterImageGenerator::
  onFitProgressEvent(::itk::Object* caller, const ::itk::EventObject& /*eventObject*/)
{
  this->InvokeEvent(::itk::ProgressEvent());

  itk::ProcessObject* process = dynamic_cast<itk::ProcessObject*>(caller);
  if (process)
  {
    this->m_Progress = process->GetProgress();
  }
};

template <typename TPixel, unsigned int VDim>
void
  mitk::PixelBasedDescriptionParameterImageGenerator::DoPrepareMask(itk::Image<TPixel, VDim>* image)
{
  m_InternalMask = dynamic_cast<InternalMaskType*>(image);

  if (m_InternalMask.IsNull())
  {
    MITK_INFO << "Parameter Fit Generator. Need to cast mask for parameter fit.";
    typedef itk::Image<TPixel, VDim> InputImageType;
    typedef itk::CastImageFilter< InputImageType, InternalMaskType > CastFilterType;
    typename CastFilterType::Pointer  spImageCaster =  CastFilterType::New();

    spImageCaster->SetInput(image);

    m_InternalMask = spImageCaster->GetOutput();
    spImageCaster->Update();
  }
}

template<typename TImage>
mitk::PixelBasedDescriptionParameterImageGenerator::ParameterImageMapType StoreResultImages(const mitk::CurveParameterFunctor::ParameterNamesType &paramNames, itk::ImageSource<TImage>* source)
{
  if (source->GetNumberOfOutputs() != paramNames.size())
  {
    mitkThrow() << "Error while generating fitted parameter images. Number of sources does not match expected parameter number. Output size: " << source->GetNumberOfOutputs() << "; number of param names: " << paramNames.size();
  }

  mitk::PixelBasedDescriptionParameterImageGenerator::ParameterImageMapType result;
  for (mitk::CurveParameterFunctor::ParameterNamesType::size_type j = 0; j < paramNames.size(); ++j)
  {
    mitk::Image::Pointer paramImage = mitk::Image::New();
    typename TImage::ConstPointer outputImg = source->GetOutput(j);
    mitk::CastToMitkImage(outputImg, paramImage);

    result.insert(std::make_pair(paramNames[j],paramImage));
  }

  return result;
}

template <typename TPixel, unsigned int VDim>
void
  mitk::PixelBasedDescriptionParameterImageGenerator::DoParameterCalculation(itk::Image<TPixel, VDim>* /*image*/)
{
  typedef itk::Image<TPixel, VDim-1> InputFrameImageType;
  typedef itk::Image<ScalarType, VDim-1> ParameterImageType;

  typedef itk::MultiOutputNaryFunctorImageFilter<InputFrameImageType, ParameterImageType, SimpleFunctorPolicy, InternalMaskType> DescriptorFilterType;

  typename DescriptorFilterType::Pointer descFilter = DescriptorFilterType::New();

  typename ::itk::MemberCommand<Self>::Pointer spProgressCommand = ::itk::MemberCommand<Self>::New();
  spProgressCommand->SetCallbackFunction(this, &Self::onFitProgressEvent);
  descFilter->AddObserver(::itk::ProgressEvent(), spProgressCommand);

  //add the time frames to the descriptor filter
  std::vector<Image::Pointer> frameCache;
  for (unsigned int i = 0; i < this->m_DynamicImage->GetTimeSteps(); ++i)
  {
    typename InputFrameImageType::Pointer frameImage;
    mitk::ImageTimeSelector::Pointer imageTimeSelector =	mitk::ImageTimeSelector::New();
    imageTimeSelector->SetInput(this->m_DynamicImage);
    imageTimeSelector->SetTimeNr(i);
    imageTimeSelector->UpdateLargestPossibleRegion();

    Image::Pointer frameMITKImage = imageTimeSelector->GetOutput();
    frameCache.push_back(frameMITKImage);
    mitk::CastToItkImage(frameMITKImage, frameImage);
    descFilter->SetInput(i,frameImage);
  }


  SimpleFunctorPolicy functor;

  functor.SetFunctor(this->m_Functor);
  descFilter->SetFunctor(functor);
  if (this->m_InternalMask.IsNotNull())
  {
    descFilter->SetMask(this->m_InternalMask);
  }

  //generate the fits
  descFilter->Update();

  //convert the outputs into mitk images and fill the parameter image map
  CurveParameterFunctor::ParameterNamesType paramNames = this->m_Functor->GetDescriptionParameterNames();

  if (descFilter->GetNumberOfOutputs() != (paramNames.size()))
  {
    mitkThrow() << "Error while generating fitted parameter images. Fit filter output size does not match expected parameter number. Output size: "<< descFilter->GetNumberOfOutputs();
  }

  this->m_TempResultMap = StoreResultImages<ParameterImageType>(paramNames,descFilter);
}

bool
  mitk::PixelBasedDescriptionParameterImageGenerator::HasOutdatedResult() const
{
  bool result = Superclass::HasOutdatedResult();

  if (m_DynamicImage.IsNotNull())
  {
    if (m_DynamicImage->GetMTime() > this->m_GenerationTimeStamp)
    {
      result = true;
    }
  }

  if (m_Mask.IsNotNull())
  {
    if (m_Mask->GetMTime() > this->m_GenerationTimeStamp)
    {
      result = true;
    }
  }

  return result;

};

void
  mitk::PixelBasedDescriptionParameterImageGenerator::CheckValidInputs() const
{
  Superclass::CheckValidInputs();

  if (m_DynamicImage.IsNull())
  {
    mitkThrow() << "Cannot generate fitted parameter images. Input dynamic image is not set.";
  }

};

void mitk::PixelBasedDescriptionParameterImageGenerator::DoParameterCalculationAndGetResults(ParameterImageMapType& parameterImages)
{
  this->m_Progress = 0;

  if(this->m_Mask.IsNotNull())
  {
    AccessFixedDimensionByItk(m_Mask, mitk::PixelBasedDescriptionParameterImageGenerator::DoPrepareMask, 3);
  }
  else
  {
    this->m_InternalMask = nullptr;
  }

  AccessFixedDimensionByItk(m_DynamicImage, mitk::PixelBasedDescriptionParameterImageGenerator::DoParameterCalculation, 4);

  parameterImages = this->m_TempResultMap;
 };

double
  mitk::PixelBasedDescriptionParameterImageGenerator::GetProgress() const
{
  return m_Progress;
};
