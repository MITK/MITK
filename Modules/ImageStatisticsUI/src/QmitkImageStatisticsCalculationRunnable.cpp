/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkImageStatisticsCalculationRunnable.h>

#include <mitkImageStatisticsCalculator.h>
#include <mitkPlanarFigure.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkImageMaskGenerator.h>
#include <mitkMultiLabelMaskGenerator.h>
#include <mitkIgnorePixelMaskGenerator.h>
#include <mitkAndMaskGenerator.h>
#include <mitkStatisticsToImageRelationRule.h>
#include <mitkStatisticsToMaskRelationRule.h>
#include <mitkImageStatisticsContainerManager.h>
#include <mitkProperties.h>

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
      nullptr == dynamic_cast<const mitk::MultiLabelSegmentation*>(mask) &&
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
  mitk::MaskGenerator::Pointer roiMaskGenerator;

  try
  {
    auto multiLabelMask = dynamic_cast<const mitk::MultiLabelSegmentation*>(m_MaskData.GetPointer());
    auto binLabelMask = dynamic_cast<const mitk::Image*>(m_MaskData.GetPointer());
    auto pfMask = dynamic_cast<const mitk::PlanarFigure*>(m_MaskData.GetPointer());

    if (nullptr != multiLabelMask)
    {
      auto multiLabelMaskGen = mitk::MultiLabelMaskGenerator::New();
      multiLabelMaskGen->SetMultiLabelSegmentation(multiLabelMask);
      roiMaskGenerator = multiLabelMaskGen;
    }
    else if (nullptr != binLabelMask)
    {
      auto imgMaskGen = mitk::ImageMaskGenerator::New();
      imgMaskGen->SetInputImage(m_StatisticsImage);
      imgMaskGen->SetImageMask(binLabelMask);
      roiMaskGenerator = imgMaskGen;
    }
    else if (nullptr != pfMask)
    {
      auto pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
      pfMaskGen->SetInputImage(m_StatisticsImage);
      pfMaskGen->SetPlanarFigure(pfMask->Clone());
      roiMaskGenerator = pfMaskGen;
    }
  }
  catch (const std::exception &e)
  {
    MITK_ERROR << "Error while configuring the statistics calculator: " << e.what();
    m_LastErrorMessage = e.what();
    statisticCalculationSuccessful = false;
  }

  mitk::MaskGenerator::Pointer maskGenerator = roiMaskGenerator;

  if (this->m_IgnoreZeros)
  {
    auto ignoreZeroMaskGen = mitk::IgnorePixelMaskGenerator::New();
    ignoreZeroMaskGen->SetIgnoredPixelValue(0);
    ignoreZeroMaskGen->SetInputImage(m_StatisticsImage);

    if (roiMaskGenerator.IsNotNull())
    {
      auto andMaskGen = mitk::AndMaskGenerator::New();
      andMaskGen->SetPrimaryMaskGenerator(roiMaskGenerator);
      andMaskGen->SetSecondaryMaskGenerator(ignoreZeroMaskGen);
      andMaskGen->SetSecondaryLabelValue(1);
      maskGenerator = andMaskGen;
    }
    else
    {
      // Without a region of interest the ignore mask is the mask itself, so the
      // statistics are reported under its label value 1.
      maskGenerator = ignoreZeroMaskGen;
    }
  }

  calculator->SetMask(maskGenerator);

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
