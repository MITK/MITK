/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageStatisticsCalculationRunnable.h"

#include "mitkImageStatisticsCalculator.h"
#include <mitkPlanarFigure.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkImageMaskGenerator.h>
#include <mitkMultiLabelMaskGenerator.h>
#include <mitkIgnorePixelMaskGenerator.h>
#include "mitkStatisticsToImageRelationRule.h"
#include "mitkStatisticsToMaskRelationRule.h"
#include "mitkImageStatisticsContainerManager.h"
#include "mitkProperties.h"

QmitkImageStatisticsCalculationRunnable::QmitkImageStatisticsCalculationRunnable()
  : QmitkDataGenerationJobBase()
  , m_StatisticsImage(nullptr)
  , m_MaskData(nullptr)
  , m_IgnoreZeros(false)
  , m_HistogramNBins(100)
{
}

QmitkImageStatisticsCalculationRunnable::~QmitkImageStatisticsCalculationRunnable()
{
}

void QmitkImageStatisticsCalculationRunnable::Initialize(const mitk::Image* image, const mitk::BaseData* mask)
{
  if (nullptr!= mask &&
      nullptr == dynamic_cast<const mitk::LabelSetImage*>(mask) &&
      nullptr == dynamic_cast<const mitk::Image*>(mask) &&
      nullptr == dynamic_cast<const mitk::PlanarFigure*>(mask))
    mitkThrow() << "Cannot initialize QmitkImageStatisticsCalculationRunnable. Mask data is not a supported type.";
  this->m_StatisticsImage = image;
  this->m_MaskData = mask;
}

mitk::ImageStatisticsContainer* QmitkImageStatisticsCalculationRunnable::GetStatisticsData() const
{
  return this->m_StatisticsContainer.GetPointer();
}

const mitk::Image* QmitkImageStatisticsCalculationRunnable::GetStatisticsImage() const
{
  return this->m_StatisticsImage.GetPointer();
}

const mitk::BaseData* QmitkImageStatisticsCalculationRunnable::GetMaskData() const
{
  return this->m_MaskData.GetPointer();
}

void QmitkImageStatisticsCalculationRunnable::SetIgnoreZeroValueVoxel(bool _arg)
{
  this->m_IgnoreZeros = _arg;
}

bool QmitkImageStatisticsCalculationRunnable::GetIgnoreZeroValueVoxel() const
{
  return this->m_IgnoreZeros;
}

void QmitkImageStatisticsCalculationRunnable::SetHistogramNBins(unsigned int nbins)
{
  this->m_HistogramNBins = nbins;
}

unsigned int QmitkImageStatisticsCalculationRunnable::GetHistogramNBins() const
{
  return this->m_HistogramNBins;
}

QmitkDataGenerationJobBase::ResultMapType QmitkImageStatisticsCalculationRunnable::GetResults() const
{
  ResultMapType result;
  result.emplace("statistics", this->GetStatisticsData());
  return result;
}

bool QmitkImageStatisticsCalculationRunnable::RunComputation()
{
  bool statisticCalculationSuccessful = true;
  mitk::ImageStatisticsCalculator::Pointer calculator = mitk::ImageStatisticsCalculator::New();

  if (this->m_StatisticsImage.IsNotNull())
  {
    calculator->SetInputImage(m_StatisticsImage);
  }
  else
  {
    statisticCalculationSuccessful = false;
  }

  // Bug 13416 : The ImageStatistics::SetImageMask() method can throw exceptions, i.e. when the dimensionality
  // of the masked and input image differ, we need to catch them and mark the calculation as failed
  // the same holds for the ::SetPlanarFigure()
  try
  {
    auto multiLabelMask = dynamic_cast<const mitk::LabelSetImage*>(m_MaskData.GetPointer());
    auto binLabelMask = dynamic_cast<const mitk::Image*>(m_MaskData.GetPointer());
    auto pfMask = dynamic_cast<const mitk::PlanarFigure*>(m_MaskData.GetPointer());

    if (nullptr != multiLabelMask)
    {
      mitk::MultiLabelMaskGenerator::Pointer imgMask = mitk::MultiLabelMaskGenerator::New();
      imgMask->SetMultiLabelSegmentation(multiLabelMask);
      calculator->SetMask(imgMask.GetPointer());
    }
    else if (nullptr != binLabelMask)
    {
      mitk::ImageMaskGenerator::Pointer imgMask = mitk::ImageMaskGenerator::New();
      imgMask->SetInputImage(m_StatisticsImage);
      imgMask->SetImageMask(binLabelMask);
      calculator->SetMask(imgMask.GetPointer());
    }
    else if (nullptr != pfMask)
    {
      mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
      pfMaskGen->SetInputImage(m_StatisticsImage);
      pfMaskGen->SetPlanarFigure(pfMask->Clone());
      calculator->SetMask(pfMaskGen.GetPointer());
    }
  }
  catch (const std::exception &e)
  {
    MITK_ERROR << "Error while configuring the statistics calculator: " << e.what();
    m_LastErrorMessage = e.what();
    statisticCalculationSuccessful = false;
  }

  if (this->m_IgnoreZeros)
  {
    mitk::IgnorePixelMaskGenerator::Pointer ignorePixelValueMaskGen = mitk::IgnorePixelMaskGenerator::New();
    ignorePixelValueMaskGen->SetIgnoredPixelValue(0);
    ignorePixelValueMaskGen->SetInputImage(m_StatisticsImage);
    calculator->SetSecondaryMask(ignorePixelValueMaskGen.GetPointer());
  }
  else
  {
    calculator->SetSecondaryMask(nullptr);
  }

  calculator->SetNBinsForHistogramStatistics(m_HistogramNBins);

  try
  {
    calculator->GetStatistics();
  }
  catch (const std::exception &e)
  {
    m_LastErrorMessage = "Failure while calculating the statistics: " + std::string(e.what());
    MITK_ERROR << m_LastErrorMessage;
    statisticCalculationSuccessful = false;
  }

  if (statisticCalculationSuccessful)
  {
    m_StatisticsContainer = calculator->GetStatistics();

    auto imageRule = mitk::StatisticsToImageRelationRule::New();
    imageRule->Connect(m_StatisticsContainer, m_StatisticsImage);

    if (nullptr != m_MaskData)
    {
      auto maskRule = mitk::StatisticsToMaskRelationRule::New();
      maskRule->Connect(m_StatisticsContainer, m_MaskData);
    }

    m_StatisticsContainer->SetProperty(mitk::STATS_HISTOGRAM_BIN_PROPERTY_NAME.c_str(), mitk::UIntProperty::New(m_HistogramNBins));
    m_StatisticsContainer->SetProperty(mitk::STATS_IGNORE_ZERO_VOXEL_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(m_IgnoreZeros));
  }
  return statisticCalculationSuccessful;
}
