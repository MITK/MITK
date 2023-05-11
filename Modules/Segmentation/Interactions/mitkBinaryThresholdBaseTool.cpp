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
  : m_SensibleMinimumThreshold(-100),
    m_SensibleMaximumThreshold(+100),
    m_LowerThreshold(1),
    m_UpperThreshold(1)
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
  if (lower < m_SensibleMinimumThreshold
    || lower > m_SensibleMaximumThreshold
    || upper < m_SensibleMinimumThreshold
    || upper > m_SensibleMaximumThreshold)
  {
    return;
  }

  m_LowerThreshold = lower;
  m_UpperThreshold = upper;

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
    m_SensibleMinimumThreshold = std::numeric_limits<ScalarType>::max();
    m_SensibleMaximumThreshold = std::numeric_limits<ScalarType>::lowest();
    Image::StatisticsHolderPointer statistics = referenceImage->GetStatistics();
    for (unsigned int ts = 0; ts < referenceImage->GetTimeSteps(); ++ts)
    {
      m_SensibleMinimumThreshold = std::min(m_SensibleMinimumThreshold, static_cast<double>(statistics->GetScalarValueMin()));
      m_SensibleMaximumThreshold = std::max(m_SensibleMaximumThreshold, static_cast<double>(statistics->GetScalarValueMax()));
    }

    if (m_LockedUpperThreshold)
    {
      m_LowerThreshold = (m_SensibleMaximumThreshold + m_SensibleMinimumThreshold) / 2.0;
      m_UpperThreshold = m_SensibleMaximumThreshold;
    }
    else
    {
      double range = m_SensibleMaximumThreshold - m_SensibleMinimumThreshold;
      m_LowerThreshold = m_SensibleMinimumThreshold + range / 3.0;
      m_UpperThreshold = m_SensibleMinimumThreshold + 2 * range / 3.0;
    }

    bool isFloatImage = false;
    if ((referenceImage->GetPixelType().GetPixelType() == itk::IOPixelEnum::SCALAR) &&
        (referenceImage->GetPixelType().GetComponentType() == itk::IOComponentEnum::FLOAT ||
          referenceImage->GetPixelType().GetComponentType() == itk::IOComponentEnum::DOUBLE))
    {
      isFloatImage = true;
    }

    IntervalBordersChanged.Send(m_SensibleMinimumThreshold, m_SensibleMaximumThreshold, isFloatImage);
    ThresholdingValuesChanged.Send(m_LowerThreshold, m_UpperThreshold);
  }
}

void mitk::BinaryThresholdBaseTool::DoUpdatePreview(const Image* inputAtTimeStep, const Image* /*oldSegAtTimeStep*/, LabelSetImage* previewImage, TimeStepType timeStep)
{
  if (nullptr != inputAtTimeStep && nullptr != previewImage)
  {
    AccessByItk_n(inputAtTimeStep, ITKThresholding, (previewImage, timeStep));
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::BinaryThresholdBaseTool::ITKThresholding(const itk::Image<TPixel, VImageDimension>* inputImage,
                                                    LabelSetImage *segmentation,
                                                    unsigned int timeStep)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<Tool::DefaultSegmentationDataType, VImageDimension> SegmentationType;
  typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;

  const auto activeValue = this->GetActiveLabelValueOfPreview();
  this->SetSelectedLabels({activeValue});

  typename ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
  filter->SetInput(inputImage);
  filter->SetLowerThreshold(m_LowerThreshold);
  filter->SetUpperThreshold(m_UpperThreshold);
  filter->SetInsideValue(activeValue);
  filter->SetOutsideValue(0);
  filter->Update();

  segmentation->SetVolume((void *)(filter->GetOutput()->GetPixelContainer()->GetBufferPointer()), timeStep);
}
