/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "itkChangeInformationImageFilter.h"

#include "mitkROIBasedParameterFitImageGenerator.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"

void
mitk::ROIBasedParameterFitImageGenerator::
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
mitk::ROIBasedParameterFitImageGenerator::DoImageGeneration(itk::Image<TPixel, VDim>* image,
    double value)
{
  typedef itk::Image<TPixel, VDim> MaskType;
  typedef itk::Image<ScalarType, VDim> ParameterImageType;

  typedef itk::ChangeInformationImageFilter< ParameterImageType > OutputImageInformationFilterType;
  typename OutputImageInformationFilterType::Pointer copyGeoInfoFilter =
    OutputImageInformationFilterType::New();

  typename ParameterImageType::Pointer paramImg = ParameterImageType::New();

  copyGeoInfoFilter->ChangeDirectionOn();
  copyGeoInfoFilter->SetOutputDirection(image->GetDirection());
  copyGeoInfoFilter->ChangeOriginOn();
  copyGeoInfoFilter->SetOutputOrigin(image->GetOrigin());
  copyGeoInfoFilter->ChangeSpacingOn();
  copyGeoInfoFilter->SetOutputSpacing(image->GetSpacing());
  copyGeoInfoFilter->SetInput(paramImg);
  copyGeoInfoFilter->Update();

  paramImg = copyGeoInfoFilter->GetOutput();
  paramImg->SetRegions(image->GetLargestPossibleRegion());
  paramImg->Allocate();
  paramImg->FillBuffer(0.0);

  typedef itk::ImageRegionConstIterator<MaskType> MaskIteratorType;
  typedef itk::ImageRegionIterator<ParameterImageType> ImageIteratorType;

  MaskIteratorType maskItr(image, image->GetLargestPossibleRegion());
  ImageIteratorType imgItr(paramImg, image->GetLargestPossibleRegion());
  maskItr.GoToBegin();
  imgItr.GoToBegin();
  while (!maskItr.IsAtEnd())
  {
    if (maskItr.Get() > 0.0)
    {
      imgItr.Set(value);
    }

    ++maskItr;
    ++imgItr;
  }

  m_TempResultImage = Image::New();
  mitk::CastToMitkImage(paramImg, m_TempResultImage);
}

bool
mitk::ROIBasedParameterFitImageGenerator::HasOutdatedResult() const
{
  bool result = Superclass::HasOutdatedResult();

  if (m_ModelParameterizer.IsNotNull())
  {
    if (m_ModelParameterizer->GetMTime() > this->m_GenerationTimeStamp)
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

  if (m_FitFunctor.IsNotNull())
  {
    if (m_FitFunctor->GetMTime() > this->m_GenerationTimeStamp)
    {
      result = true;
    }
  }

  return result;
};

void
mitk::ROIBasedParameterFitImageGenerator::CheckValidInputs() const
{
  Superclass::CheckValidInputs();

  if (m_Mask.IsNull())
  {
    mitkThrow() << "Cannot generate fitted parameter images. Input mask is not set.";
  }

  if (m_Signal.Size() != m_TimeGrid.Size())
  {
    mitkThrow() << "Cannot generate fitted parameter images. Signal and TimeGrid do not match.";
  }

};

void mitk::ROIBasedParameterFitImageGenerator::DoFitAndGetResults(ParameterImageMapType&
    parameterImages, ParameterImageMapType& derivedParameterImages,
    ParameterImageMapType& criterionImages, ParameterImageMapType& evaluationParameterImages)
{
  this->m_Progress = 0;

  //fit the signal
  ModelParameterizerBase::IndexType index;
  index.Fill(0);
  this->m_ModelParameterizer->SetDefaultTimeGrid(m_TimeGrid);
  ParameterizerType::ModelBasePointer parameterizedModel =
    m_ModelParameterizer->GenerateParameterizedModel(index);
  ParameterizerType::ParametersType initialParameters =
    m_ModelParameterizer->GetInitialParameterization(index);

  ModelFitFunctorBase::InputPixelArrayType inputValues;

  for (SignalType::const_iterator pos = m_Signal.begin(); pos != m_Signal.end(); ++pos)
  {
    inputValues.push_back(*pos);
  }

  ModelFitFunctorBase::OutputPixelArrayType fitResult = m_FitFunctor->Compute(inputValues,
      parameterizedModel, initialParameters);

  //generate the results maps
  ParameterImageMapType tempResultMap;
  ParameterImageMapType tempDerivedResultMap;
  ParameterImageMapType tempEvaluationResultMap;
  ParameterImageMapType tempCriterionResultMap;

  ModelFitFunctorBase::ParameterNamesType paramNames = parameterizedModel->GetParameterNames();
  ModelFitFunctorBase::ParameterNamesType derivedParamNames =
    parameterizedModel->GetDerivedParameterNames();
  ModelFitFunctorBase::ParameterNamesType criterionNames = this->m_FitFunctor->GetCriterionNames();
  ModelFitFunctorBase::ParameterNamesType evaluationParamNames =
    this->m_FitFunctor->GetEvaluationParameterNames();
  ModelFitFunctorBase::ParameterNamesType debugParamNames = this->m_FitFunctor->GetDebugParameterNames();

  if (fitResult.size() != (paramNames.size() + derivedParamNames.size() + criterionNames.size() +
                           evaluationParamNames.size() + debugParamNames.size()))
  {
    mitkThrow() <<
                "Error while generating fitted parameter images. Fit functor output size does not match expected parameter number. Output size: "
                << fitResult.size();
  }

  for (ModelFitFunctorBase::ParameterNamesType::size_type j = 0; j < paramNames.size(); ++j)
  {
    ModelFitFunctorBase::OutputPixelArrayType::value_type value = fitResult[j];
    AccessByItk_n(m_Mask, mitk::ROIBasedParameterFitImageGenerator::DoImageGeneration, (value));
    tempResultMap.insert(std::make_pair(paramNames[j], m_TempResultImage));
  }

  ModelFitFunctorBase::ParameterNamesType::size_type offset = paramNames.size();
  for (ModelFitFunctorBase::ParameterNamesType::size_type j = 0; j < derivedParamNames.size(); ++j)
  {
    ModelFitFunctorBase::OutputPixelArrayType::value_type value = fitResult[j + offset];
    AccessByItk_n(m_Mask, mitk::ROIBasedParameterFitImageGenerator::DoImageGeneration, (value));
    tempDerivedResultMap.insert(std::make_pair(derivedParamNames[j], m_TempResultImage));
  }

  offset += derivedParamNames.size();
  for (ModelFitFunctorBase::ParameterNamesType::size_type j = 0; j < criterionNames.size(); ++j)
  {
    ModelFitFunctorBase::OutputPixelArrayType::value_type value = fitResult[j + offset];
    AccessByItk_n(m_Mask, mitk::ROIBasedParameterFitImageGenerator::DoImageGeneration, (value));
    tempCriterionResultMap.insert(std::make_pair(criterionNames[j], m_TempResultImage));
  }

  offset += criterionNames.size();
  for (ModelFitFunctorBase::ParameterNamesType::size_type j = 0; j < evaluationParamNames.size(); ++j)
  {
    ModelFitFunctorBase::OutputPixelArrayType::value_type value = fitResult[j + offset];
    AccessByItk_n(m_Mask, mitk::ROIBasedParameterFitImageGenerator::DoImageGeneration, (value));
    tempEvaluationResultMap.insert(std::make_pair(evaluationParamNames[j], m_TempResultImage));
  }

  offset += evaluationParamNames.size();
  for (ModelFitFunctorBase::ParameterNamesType::size_type j = 0; j < debugParamNames.size(); ++j)
  { //add debug params (if they are generated to the evaluation result map
    ModelFitFunctorBase::OutputPixelArrayType::value_type value = fitResult[j + offset];
    AccessByItk_n(m_Mask, mitk::ROIBasedParameterFitImageGenerator::DoImageGeneration, (value));
    tempEvaluationResultMap.insert(std::make_pair(debugParamNames[j], m_TempResultImage));
  }

  parameterImages = tempResultMap;
  derivedParameterImages = tempDerivedResultMap;
  criterionImages = tempCriterionResultMap;
  evaluationParameterImages = tempEvaluationResultMap;

};

double
mitk::ROIBasedParameterFitImageGenerator::GetProgress() const
{
  return m_Progress;
};

mitk::ROIBasedParameterFitImageGenerator::ParameterNamesType
mitk::ROIBasedParameterFitImageGenerator::GetParameterNames() const
{
  ParameterizerType::ModelBasePointer parameterizedModel =
    m_ModelParameterizer->GenerateParameterizedModel();

  return parameterizedModel->GetParameterNames();
}

mitk::ROIBasedParameterFitImageGenerator::ParameterNamesType
mitk::ROIBasedParameterFitImageGenerator::GetDerivedParameterNames() const
{
  ParameterizerType::ModelBasePointer parameterizedModel =
    m_ModelParameterizer->GenerateParameterizedModel();

  return parameterizedModel->GetDerivedParameterNames();
}

mitk::ROIBasedParameterFitImageGenerator::ParameterNamesType
mitk::ROIBasedParameterFitImageGenerator::GetCriterionNames() const
{
  return this->m_FitFunctor->GetCriterionNames();
}

mitk::ROIBasedParameterFitImageGenerator::ParameterNamesType
mitk::ROIBasedParameterFitImageGenerator::GetEvaluationParameterNames() const
{
  auto evals = this->m_FitFunctor->GetEvaluationParameterNames();
  auto debugs = this->m_FitFunctor->GetDebugParameterNames();

  evals.insert(evals.end(), debugs.begin(), debugs.end());

  return evals;
}
