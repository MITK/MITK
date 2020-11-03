/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBinaryThresholdBaseTool.h"

#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkLabelSetImage.h"
#include <itkBinaryThresholdImageFilter.h>
#include <itkImageRegionIterator.h>

mitk::BinaryThresholdBaseTool::BinaryThresholdBaseTool()
  : m_SensibleMinimumThresholdValue(-100),
    m_SensibleMaximumThresholdValue(+100),
    m_CurrentLowerThresholdValue(1),
    m_CurrentUpperThresholdValue(1)
{
}

mitk::BinaryThresholdBaseTool::~BinaryThresholdBaseTool()
{
}

void mitk::BinaryThresholdBaseTool::SetThresholdValues(double lower, double upper)
{
  /* If value is not in the min/max range, do nothing. In that case, this
     method will be called again with a proper value right after. The only
     known case where this happens is with an [0.0, 1.0[ image, where value
     could be an epsilon greater than the max. */
  if (lower < m_SensibleMinimumThresholdValue
    || lower > m_SensibleMaximumThresholdValue
    || upper < m_SensibleMinimumThresholdValue
    || upper > m_SensibleMaximumThresholdValue)
  {
    return;
  }

  m_CurrentLowerThresholdValue = lower;
  m_CurrentUpperThresholdValue = upper;

  if (nullptr != this->GetPreviewSegmentation())
  {
    UpdatePreview();
  }
}

void mitk::BinaryThresholdBaseTool::InitiateToolByInput()
{
  const auto referenceImage = this->GetReferenceData();
  if (nullptr != referenceImage)
  {
    m_SensibleMinimumThresholdValue = std::numeric_limits<ScalarType>::max();
    m_SensibleMaximumThresholdValue = std::numeric_limits<ScalarType>::lowest();
    Image::StatisticsHolderPointer statistics = referenceImage->GetStatistics();
    for (unsigned int ts = 0; ts < referenceImage->GetTimeSteps(); ++ts)
    {
      m_SensibleMinimumThresholdValue = std::min(m_SensibleMinimumThresholdValue, static_cast<double>(statistics->GetScalarValueMin()));
      m_SensibleMaximumThresholdValue = std::max(m_SensibleMaximumThresholdValue, static_cast<double>(statistics->GetScalarValueMax()));
    }

    if (m_LockedUpperThreshold)
    {
      m_CurrentLowerThresholdValue = (m_SensibleMaximumThresholdValue + m_SensibleMinimumThresholdValue) / 2.0;
      m_CurrentUpperThresholdValue = m_SensibleMaximumThresholdValue;
    }
    else
    {
      double range = m_SensibleMaximumThresholdValue - m_SensibleMinimumThresholdValue;
      m_CurrentLowerThresholdValue = m_SensibleMinimumThresholdValue + range / 3.0;
      m_CurrentUpperThresholdValue = m_SensibleMinimumThresholdValue + 2 * range / 3.0;
    }

    bool isFloatImage = false;
    if ((referenceImage->GetPixelType().GetPixelType() == itk::ImageIOBase::SCALAR) &&
        (referenceImage->GetPixelType().GetComponentType() == itk::ImageIOBase::FLOAT ||
          referenceImage->GetPixelType().GetComponentType() == itk::ImageIOBase::DOUBLE))
    {
      isFloatImage = true;
    }

    IntervalBordersChanged.Send(m_SensibleMinimumThresholdValue, m_SensibleMaximumThresholdValue, isFloatImage);
    ThresholdingValuesChanged.Send(m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue);
  }
}

template <typename TPixel, unsigned int VImageDimension>
static void ITKThresholding(const itk::Image<TPixel, VImageDimension> *originalImage,
                            mitk::Image *segmentation,
                            double lower,
                            double upper,
                            unsigned int timeStep)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, VImageDimension> SegmentationType;
  typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;

  typename ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
  filter->SetInput(originalImage);
  filter->SetLowerThreshold(lower);
  filter->SetUpperThreshold(upper);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->Update();

  segmentation->SetVolume((void *)(filter->GetOutput()->GetPixelContainer()->GetBufferPointer()), timeStep);
}

void mitk::BinaryThresholdBaseTool::DoUpdatePreview(const Image* inputAtTimeStep, Image* previewImage, TimeStepType timeStep)
{
  if (nullptr != inputAtTimeStep && nullptr != previewImage)
  {
      AccessByItk_n(inputAtTimeStep,
        ITKThresholding,
        (previewImage, m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue, timeStep));
  }
}
