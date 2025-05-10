/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBooleanOperation.h"
#include <itkNaryFunctorImageFilter.h>
#include <mitkExceptionMacro.h>
#include <mitkImageCast.h>
#include <mitkImageTimeSelector.h>
#include <mitkITKEventObserverGuard.h>

using ITKGroupImageType = itk::Image<mitk::Label::PixelType, 3>;
using BoolOpsFunctionType = mitk::Label::PixelType(const std::vector<mitk::Label::PixelType>& inputArray);

/** Functor class that implements the label transfer and is used in conjunction with the itk::BinaryFunctorImageFilter.
* For details regarding the usage of the filter and the functor patterns, please see info of itk::BinaryFunctorImageFilter.
*/
template <class TPixelType>
class BoolOpsFunctor
{

public:
  BoolOpsFunctor() {};

  BoolOpsFunctor(std::function<BoolOpsFunctionType> opsFunction) : m_Function(opsFunction)
  {
  };

  ~BoolOpsFunctor() {};

  bool operator!=(const BoolOpsFunctor& other)const
  {
    return !(*this == other);
  }

  bool operator==(const BoolOpsFunctor& other) const
  {
    if ((this->m_Function.target_type() == other.m_Function.target_type())
      && (this->m_Function.template target<BoolOpsFunctionType>() && other.m_Function.template target<BoolOpsFunctionType>())
      // If both std::function objects hold function pointers of the same signature,
      // we can compare the pointers to check if they point to the same function.
      && (*this->m_Function.template target<BoolOpsFunctionType>() == *other.m_Function.template target<BoolOpsFunctionType>()))
    {
      return true;
    }
    return false;
  }

  BoolOpsFunctor& operator=(const BoolOpsFunctor& other)
  {
    this->m_Function = other.m_Function;

    return *this;
  }

  inline TPixelType operator()(const std::vector<TPixelType>& inputArray)
  {
    return m_Function(inputArray);
  }

private:
  std::function<BoolOpsFunctionType> m_Function;
};


mitk::Image::Pointer GenerateInternal(const mitk::MultiLabelSegmentation* segmentation, mitk::MultiLabelSegmentation::LabelValueVectorType labelValues, std::function<BoolOpsFunctionType> opsFunction,
  std::function<void(float progress)> progressCallback = [](float) {})
{
  if (nullptr == segmentation) mitkThrow() << "Cannot perform boolean operation. Passed segmentation is not valid";

  mitk::Image::Pointer result = mitk::Image::New();
  result->Initialize(mitk::MakeScalarPixelType<mitk::MultiLabelSegmentation::LabelValueType>(), *(segmentation->GetTimeGeometry()));

  const auto timeStepCount = segmentation->GetTimeGeometry()->CountTimeSteps();

  for (mitk::TimeStepType i = 0; i < timeStepCount; ++i)
  {
    using OpsFilterType = itk::NaryFunctorImageFilter<ITKGroupImageType, ITKGroupImageType, BoolOpsFunctor<mitk::MultiLabelSegmentation::LabelValueType> >;
    auto opsFilter = OpsFilterType::New();

    mitk::ITKEventObserverGuard eventGuard(opsFilter, itk::ProgressEvent(), [&opsFilter, progressCallback, i, timeStepCount](const itk::EventObject& /*event*/)
      { progressCallback(opsFilter->GetProgress() + static_cast<float>(i) / static_cast<float>(timeStepCount)); });

    BoolOpsFunctor<mitk::MultiLabelSegmentation::LabelValueType> functor(opsFunction);
    opsFilter->SetFunctor(functor);
    std::vector < ITKGroupImageType::ConstPointer > inputImages;

    unsigned int inputIndex = 0;
    for (auto value : labelValues)
    {
      auto groupImage = segmentation->GetGroupImage(segmentation->GetGroupIndexOfLabel(value));
      auto groupImageAtTS = mitk::SelectImageByTimeStep(groupImage, i);

      ITKGroupImageType::Pointer itkImage;
      mitk::CastToItkImage(groupImageAtTS, itkImage);
      inputImages.push_back(itkImage);
      opsFilter->SetInput(inputIndex, itkImage);
      ++inputIndex;
    }

    opsFilter->Update();
    auto resultTS = opsFilter->GetOutput();

    result->SetVolume(resultTS->GetBufferPointer(), i);
  }

  return result;
}


mitk::Image::Pointer mitk::BooleanOperation::GenerateUnion(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::LabelValueVectorType labelValues,
  std::function<void(float progress)> progressCallback)
{
  auto unionOps = [labelValues](const std::vector<mitk::Label::PixelType>& inputArray)
    {
      mitk::Label::PixelType result = 0;
      for (auto inIt = inputArray.cbegin(), refIt = labelValues.cbegin(); inIt != inputArray.cend(); ++inIt, ++refIt)
      {
        if (*inIt == *refIt)
        {
          result = 1;
          break;
        }
      }
      return result;
    };

  return GenerateInternal(segmentation, labelValues, unionOps, progressCallback);
}

mitk::Image::Pointer mitk::BooleanOperation::GenerateIntersection(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::LabelValueVectorType labelValues,
  std::function<void(float progress)> progressCallback)
{
  auto intersectOps = [labelValues](const std::vector<mitk::Label::PixelType>& inputArray)
    {
      mitk::Label::PixelType result = 1;
      for (auto inIt = inputArray.cbegin(), refIt = labelValues.cbegin(); inIt != inputArray.cend(); ++inIt, ++refIt)
      {
        if (*inIt != *refIt)
        {
          result = 0;
          break;
        }
      }
      return result;
    };

  return GenerateInternal(segmentation, labelValues, intersectOps, progressCallback);
}

mitk::Image::Pointer mitk::BooleanOperation::GenerateDifference(const MultiLabelSegmentation* segmentation, MultiLabelSegmentation::LabelValueType minuendLabelValue,
  const MultiLabelSegmentation::LabelValueVectorType subtrahendLabelValues, std::function<void(float progress)> progressCallback)
{
  auto intersectOps = [minuendLabelValue, subtrahendLabelValues](const std::vector<mitk::Label::PixelType>& inputArray)
    {
      if (minuendLabelValue != inputArray.front())
        return mitk::Label::PixelType(0);

      mitk::Label::PixelType result = 1;
      for (auto inIt = inputArray.cbegin()+1, refIt = subtrahendLabelValues.cbegin(); inIt != inputArray.cend(); ++inIt, ++refIt)
      {
        if (*inIt == *refIt)
        {
          result = 0;
          break;
        }
      }
      return result;
    };

  auto labelValues = subtrahendLabelValues;
  labelValues.insert(labelValues.begin(), minuendLabelValue);
  return GenerateInternal(segmentation, labelValues, intersectOps, progressCallback);
}
