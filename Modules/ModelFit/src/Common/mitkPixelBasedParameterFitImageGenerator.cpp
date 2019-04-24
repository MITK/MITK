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

#include "itkCommand.h"
#include "itkMultiOutputNaryFunctorImageFilter.h"

#include "mitkPixelBasedParameterFitImageGenerator.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkModelFitFunctorPolicy.h"

#include "mitkExtractTimeGrid.h"

void
  mitk::PixelBasedParameterFitImageGenerator::
  onFitProgressEvent(::itk::Object* caller, const ::itk::EventObject& /*eventObject*/)
{
  this->InvokeEvent(::itk::ProgressEvent());

  auto* process = dynamic_cast<itk::ProcessObject*>(caller);
  if (process)
  {
    this->m_Progress = process->GetProgress();
  }
};

template <typename TPixel, unsigned int VDim>
void 
  mitk::PixelBasedParameterFitImageGenerator::DoPrepareMask(itk::Image<TPixel, VDim>* image)
{
  m_InternalMask = dynamic_cast<InternalMaskType*>(image);

  if (m_InternalMask.IsNull())
  {
    MITK_INFO << "Parameter Fit Generator. Need to cast mask for parameter fit.";
    using InputImageType = itk::Image<TPixel, VDim>;
    using CastFilterType = itk::CastImageFilter< InputImageType, InternalMaskType >;
    typename CastFilterType::Pointer  spImageCaster =  CastFilterType::New();

    spImageCaster->SetInput(image);

    m_InternalMask = spImageCaster->GetOutput();
    spImageCaster->Update();
  }
}

template<typename TImage>
mitk::PixelBasedParameterFitImageGenerator::ParameterImageMapType StoreResultImages( mitk::ModelFitFunctorBase::ParameterNamesType &paramNames, itk::ImageSource<TImage>* source, mitk::ModelFitFunctorBase::ParameterNamesType::size_type startPos, mitk::ModelFitFunctorBase::ParameterNamesType::size_type& endPos ) 
{
  mitk::PixelBasedParameterFitImageGenerator::ParameterImageMapType result;
  for (mitk::ModelFitFunctorBase::ParameterNamesType::size_type j = 0; j < paramNames.size(); ++j)
  {
    if (source->GetNumberOfOutputs() < startPos+j)
    {
      mitkThrow() << "Error while generating fitted parameter images. Number of sources is too low and does not match expected parameter number. Output size: "<< source->GetNumberOfOutputs()<<"; number of param names: "<<paramNames.size()<<";source start pos: " << startPos;
    }

    mitk::Image::Pointer paramImage = mitk::Image::New();
    typename TImage::ConstPointer outputImg = source->GetOutput(startPos+j);
    mitk::CastToMitkImage(outputImg, paramImage);

    result.insert(std::make_pair(paramNames[j],paramImage));
  }

  endPos = startPos + paramNames.size();

  return result;
}

template <typename TPixel, unsigned int VDim>
void 
  mitk::PixelBasedParameterFitImageGenerator::DoParameterFit(itk::Image<TPixel, VDim>* /*image*/)
{
  using InputFrameImageType = itk::Image<TPixel, VDim-1>;
  using ParameterImageType = itk::Image<ScalarType, VDim-1>;

  using FitFilterType = itk::MultiOutputNaryFunctorImageFilter<InputFrameImageType, ParameterImageType, ModelFitFunctorPolicy, InternalMaskType>;

  typename FitFilterType::Pointer fitFilter = FitFilterType::New();

  typename ::itk::MemberCommand<Self>::Pointer spProgressCommand = ::itk::MemberCommand<Self>::New();
  spProgressCommand->SetCallbackFunction(this, &Self::onFitProgressEvent);
  fitFilter->AddObserver(::itk::ProgressEvent(), spProgressCommand);

  //add the time frames to the fit filter
  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
  imageTimeSelector->SetInput(this->m_DynamicImage);
  std::vector<Image::Pointer> frameCache;
  for (unsigned int i = 0; i < this->m_DynamicImage->GetTimeSteps(); ++i)
  {
    typename InputFrameImageType::Pointer frameImage;
    imageTimeSelector->SetTimeNr(i);
    imageTimeSelector->UpdateLargestPossibleRegion();

    Image::Pointer frameMITKImage = imageTimeSelector->GetOutput();
    frameCache.push_back(frameMITKImage);
    mitk::CastToItkImage(frameMITKImage, frameImage);
    fitFilter->SetInput(i,frameImage);
  }

  ModelBaseType::TimeGridType timeGrid = ExtractTimeGrid(m_DynamicImage);
  if (m_TimeGridByParameterizer)
  {
    if (timeGrid.GetSize() != m_ModelParameterizer->GetDefaultTimeGrid().GetSize())
    {
      mitkThrow() << "Cannot do fitting. Filter is set to use default time grid of the parameterizer, but grid size does not match the number of input image frames. Grid size: " << m_ModelParameterizer->GetDefaultTimeGrid().GetSize() << "; frame count: " << timeGrid.GetSize();
    }

  }
  else
  {
    this->m_ModelParameterizer->SetDefaultTimeGrid(timeGrid);
  }

  ModelFitFunctorPolicy functor;

  functor.SetModelFitFunctor(this->m_FitFunctor); 
  functor.SetModelParameterizer(this->m_ModelParameterizer);
  fitFilter->SetFunctor(functor);
  if (this->m_InternalMask.IsNotNull())
  {
    fitFilter->SetMask(this->m_InternalMask);
  }

  //generate the fits
  fitFilter->Update();

  //convert the outputs into mitk images and fill the parameter image map
  ModelBaseType::Pointer refModel = this->m_ModelParameterizer->GenerateParameterizedModel();
  ModelFitFunctorBase::ParameterNamesType paramNames = refModel->GetParameterNames();
  ModelFitFunctorBase::ParameterNamesType derivedParamNames = refModel->GetDerivedParameterNames();
  ModelFitFunctorBase::ParameterNamesType criterionNames = this->m_FitFunctor->GetCriterionNames();
  ModelFitFunctorBase::ParameterNamesType evaluationParamNames = this->m_FitFunctor->GetEvaluationParameterNames();
  ModelFitFunctorBase::ParameterNamesType debugParamNames = this->m_FitFunctor->GetDebugParameterNames();

  if (fitFilter->GetNumberOfOutputs() != (paramNames.size() + derivedParamNames.size() + criterionNames.size() + evaluationParamNames.size() + debugParamNames.size()))
  {
    mitkThrow() << "Error while generating fitted parameter images. Fit filter output size does not match expected parameter number. Output size: "<< fitFilter->GetNumberOfOutputs();
  }

  ModelFitFunctorBase::ParameterNamesType::size_type resultPos = 0;
  this->m_TempResultMap = StoreResultImages<ParameterImageType>(paramNames,fitFilter,resultPos, resultPos);
  this->m_TempDerivedResultMap = StoreResultImages<ParameterImageType>(derivedParamNames,fitFilter,resultPos, resultPos);
  this->m_TempCriterionResultMap = StoreResultImages<ParameterImageType>(criterionNames,fitFilter,resultPos, resultPos);
  this->m_TempEvaluationResultMap = StoreResultImages<ParameterImageType>(evaluationParamNames,fitFilter,resultPos, resultPos);
  //also add debug params (if generated) to the evaluation result map
  mitk::PixelBasedParameterFitImageGenerator::ParameterImageMapType debugMap = StoreResultImages<ParameterImageType>(debugParamNames, fitFilter, resultPos, resultPos);
  this->m_TempEvaluationResultMap.insert(debugMap.begin(), debugMap.end());
}

bool
  mitk::PixelBasedParameterFitImageGenerator::HasOutdatedResult() const
{
  bool result = Superclass::HasOutdatedResult();

  if (m_ModelParameterizer.IsNotNull())
  {
    if (m_ModelParameterizer->GetMTime() > this->m_GenerationTimeStamp)
    {
      result = true;
    }
  }

  if (m_FitFunctor.IsNotNull())
  {
    if (m_FitFunctor->GetMTime() > this->m_GenerationTimeStamp)
    {
      result = true;
    }
  }

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
  mitk::PixelBasedParameterFitImageGenerator::CheckValidInputs() const
{
  Superclass::CheckValidInputs();

  if (m_DynamicImage.IsNull())
  {
    mitkThrow() << "Cannot generate fitted parameter images. Input dynamic image is not set.";
  }

};

void mitk::PixelBasedParameterFitImageGenerator::DoFitAndGetResults(ParameterImageMapType& parameterImages, ParameterImageMapType& derivedParameterImages, ParameterImageMapType& criterionImages, ParameterImageMapType& evaluationParameterImages)
{
  this->m_Progress = 0;

  if(this->m_Mask.IsNotNull())
  {
    AccessFixedDimensionByItk(m_Mask, mitk::PixelBasedParameterFitImageGenerator::DoPrepareMask, 3);
  }
  else
  {
    this->m_InternalMask = nullptr;
  }

  AccessFixedDimensionByItk(m_DynamicImage, mitk::PixelBasedParameterFitImageGenerator::DoParameterFit, 4);

  parameterImages = this->m_TempResultMap;
  derivedParameterImages = this->m_TempDerivedResultMap;
  criterionImages = this->m_TempCriterionResultMap;
  evaluationParameterImages = this->m_TempEvaluationResultMap;

};

double
  mitk::PixelBasedParameterFitImageGenerator::GetProgress() const
{
  return m_Progress;
};

mitk::PixelBasedParameterFitImageGenerator::ParameterNamesType
mitk::PixelBasedParameterFitImageGenerator::GetParameterNames() const
{
  ParameterizerType::ModelBasePointer parameterizedModel =
    m_ModelParameterizer->GenerateParameterizedModel();

  return parameterizedModel->GetParameterNames();
}

mitk::PixelBasedParameterFitImageGenerator::ParameterNamesType
mitk::PixelBasedParameterFitImageGenerator::GetDerivedParameterNames() const
{
  ParameterizerType::ModelBasePointer parameterizedModel =
    m_ModelParameterizer->GenerateParameterizedModel();

  return parameterizedModel->GetDerivedParameterNames();
}

mitk::PixelBasedParameterFitImageGenerator::ParameterNamesType
mitk::PixelBasedParameterFitImageGenerator::GetCriterionNames() const
{
  return this->m_FitFunctor->GetCriterionNames();
}

mitk::PixelBasedParameterFitImageGenerator::ParameterNamesType
mitk::PixelBasedParameterFitImageGenerator::GetEvaluationParameterNames() const
{
  auto evals = this->m_FitFunctor->GetEvaluationParameterNames();
  auto debugs = this->m_FitFunctor->GetDebugParameterNames();

  evals.insert(evals.end(), debugs.begin(), debugs.end());

  return evals;
}
