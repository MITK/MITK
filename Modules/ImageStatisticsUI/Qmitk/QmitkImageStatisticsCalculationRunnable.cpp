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

#include "QmitkImageStatisticsCalculationRunnable.h"

#include "mitkImageStatisticsCalculator.h"
#include <mitkImageMaskGenerator.h>
#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkIgnorePixelMaskGenerator.h>
#include "mitkStatisticsToImageRelationRule.h"
#include "mitkStatisticsToMaskRelationRule.h"
#include "mitkImageStatisticsContainerManager.h"
#include "mitkProperties.h"

QmitkImageStatisticsCalculationRunnable::QmitkImageStatisticsCalculationRunnable()
  : QmitkDataGenerationJobBase()
  , m_StatisticsImage(nullptr)
  , m_BinaryMask(nullptr)
  , m_PlanarFigureMask(nullptr)
  , m_IgnoreZeros(false)
  , m_HistogramNBins(100)
{
}

QmitkImageStatisticsCalculationRunnable::~QmitkImageStatisticsCalculationRunnable()
{
}

void QmitkImageStatisticsCalculationRunnable::Initialize(const mitk::Image *image,
  const mitk::Image *binaryImage,
  const mitk::PlanarFigure *planarFig)
{
  this->m_StatisticsImage = image;
  this->m_BinaryMask = binaryImage;
  this->m_PlanarFigureMask = planarFig;
}

mitk::ImageStatisticsContainer* QmitkImageStatisticsCalculationRunnable::GetStatisticsData() const
{
  return this->m_StatisticsContainer.GetPointer();
}

const mitk::Image* QmitkImageStatisticsCalculationRunnable::GetStatisticsImage() const
{
  return this->m_StatisticsImage.GetPointer();
}

const mitk::Image* QmitkImageStatisticsCalculationRunnable::GetMaskImage() const
{
  return this->m_BinaryMask.GetPointer();
}

const mitk::PlanarFigure* QmitkImageStatisticsCalculationRunnable::GetPlanarFigure() const
{
  return this->m_PlanarFigureMask.GetPointer();
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
    if (this->m_BinaryMask.IsNotNull())
    {
      mitk::ImageMaskGenerator::Pointer imgMask = mitk::ImageMaskGenerator::New();
      imgMask->SetImageMask(m_BinaryMask->Clone());
      calculator->SetMask(imgMask.GetPointer());
    }
    if (this->m_PlanarFigureMask.IsNotNull())
    {
      mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
      pfMaskGen->SetInputImage(m_StatisticsImage);
      pfMaskGen->SetPlanarFigure(m_PlanarFigureMask->Clone());
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

    if (nullptr != m_PlanarFigureMask)
    {
      auto maskRule = mitk::StatisticsToMaskRelationRule::New();
      maskRule->Connect(m_StatisticsContainer, m_PlanarFigureMask);
    }

    if (nullptr != m_BinaryMask)
    {
      auto maskRule = mitk::StatisticsToMaskRelationRule::New();
      maskRule->Connect(m_StatisticsContainer, m_BinaryMask);
    }

    m_StatisticsContainer->SetProperty(mitk::STATS_HISTOGRAM_BIN_PROPERTY_NAME.c_str(), mitk::UIntProperty::New(m_HistogramNBins));
    m_StatisticsContainer->SetProperty(mitk::STATS_IGNORE_ZERO_VOXEL_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(m_IgnoreZeros));
  }
  return statisticCalculationSuccessful;
}
