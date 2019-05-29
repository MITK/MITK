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

#include "QmitkImageStatisticsCalculationJob.h"

#include "mitkImageStatisticsCalculator.h"
#include <mitkImageMaskGenerator.h>
#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkIgnorePixelMaskGenerator.h>

QmitkImageStatisticsCalculationJob::QmitkImageStatisticsCalculationJob()
  : QThread()
  , m_StatisticsImage(nullptr)
  , m_BinaryMask(nullptr)
  , m_PlanarFigureMask(nullptr)
  , m_IgnoreZeros(false)
  , m_HistogramNBins(100)
  , m_CalculationSuccessful(false)
{
}

QmitkImageStatisticsCalculationJob::~QmitkImageStatisticsCalculationJob()
{
}

void QmitkImageStatisticsCalculationJob::Initialize(const mitk::Image *image,
                                                    const mitk::Image *binaryImage,
                                                    const mitk::PlanarFigure *planarFig)
{
  this->m_StatisticsImage = image;
  this->m_BinaryMask = binaryImage;
  this->m_PlanarFigureMask = planarFig;
}

mitk::ImageStatisticsContainer* QmitkImageStatisticsCalculationJob::GetStatisticsData() const
{
  return this->m_StatisticsContainer.GetPointer();
}

const mitk::Image* QmitkImageStatisticsCalculationJob::GetStatisticsImage() const
{
  return this->m_StatisticsImage.GetPointer();
}

const mitk::Image* QmitkImageStatisticsCalculationJob::GetMaskImage() const
{
  return this->m_BinaryMask.GetPointer();
}

const mitk::PlanarFigure* QmitkImageStatisticsCalculationJob::GetPlanarFigure() const
{
  return this->m_PlanarFigureMask.GetPointer();
}

void QmitkImageStatisticsCalculationJob::SetIgnoreZeroValueVoxel(bool _arg)
{
  this->m_IgnoreZeros = _arg;
}

bool QmitkImageStatisticsCalculationJob::GetIgnoreZeroValueVoxel() const
{
  return this->m_IgnoreZeros;
}

void QmitkImageStatisticsCalculationJob::SetHistogramNBins(unsigned int nbins)
{
  this->m_HistogramNBins = nbins;
}

unsigned int QmitkImageStatisticsCalculationJob::GetHistogramNBins() const
{
  return this->m_HistogramNBins;
}

std::string QmitkImageStatisticsCalculationJob::GetLastErrorMessage() const
{
  return m_message;
}

const QmitkImageStatisticsCalculationJob::HistogramType*
QmitkImageStatisticsCalculationJob::GetTimeStepHistogram(unsigned int t) const
{
  if (t >= this->m_HistogramVector.size())
    return nullptr;

  return this->m_HistogramVector.at(t).GetPointer();
}

bool QmitkImageStatisticsCalculationJob::GetStatisticsUpdateSuccessFlag() const
{
  return m_CalculationSuccessful;
}

void QmitkImageStatisticsCalculationJob::run()
{
  bool statisticCalculationSuccessful = true;
  mitk::ImageStatisticsCalculator::Pointer calculator = mitk::ImageStatisticsCalculator::New();

  if(this->m_StatisticsImage.IsNotNull())
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
    if(this->m_BinaryMask.IsNotNull())
    {
      mitk::ImageMaskGenerator::Pointer imgMask = mitk::ImageMaskGenerator::New();
      imgMask->SetImageMask(m_BinaryMask->Clone());
      calculator->SetMask(imgMask.GetPointer());
    }
    if(this->m_PlanarFigureMask.IsNotNull())
    {
      mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
      pfMaskGen->SetInputImage(m_StatisticsImage);
      pfMaskGen->SetPlanarFigure(m_PlanarFigureMask->Clone());
      calculator->SetMask(pfMaskGen.GetPointer());
    }
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "MITK Exception: " << e.what();
    m_message = e.what();
    statisticCalculationSuccessful = false;
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_ERROR << "ITK Exception:" << e.what();
    m_message = e.what();
    statisticCalculationSuccessful = false;
  }
  catch ( const std::runtime_error &e )
  {
    MITK_ERROR<< "Runtime Exception: " << e.what();
    m_message = e.what();
    statisticCalculationSuccessful = false;
  }
  catch ( const std::exception &e )
  {
    MITK_ERROR<< "Standard Exception: " << e.what();
    m_message = e.what();
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
    catch ( mitk::Exception& e)
    {
      m_message = e.GetDescription();
      MITK_ERROR<< "MITK Exception: " << e.what();
      statisticCalculationSuccessful = false;
    }
    catch ( const std::runtime_error &e )
    {
      m_message = "Failure: " + std::string(e.what());
      MITK_ERROR<< "Runtime Exception: " << e.what();
      statisticCalculationSuccessful = false;
    }
    catch ( const std::exception &e )
    {
      m_message = "Failure: " + std::string(e.what());
      MITK_ERROR<< "Standard Exception: " << e.what();
      statisticCalculationSuccessful = false;
    }

  this->m_CalculationSuccessful = statisticCalculationSuccessful;

  if(statisticCalculationSuccessful)
  {
    m_StatisticsContainer = calculator->GetStatistics();
    this->m_HistogramVector.clear();

    for (unsigned int i = 0; i < m_StatisticsImage->GetTimeSteps(); i++)
    {
      HistogramType::ConstPointer tempHistogram;
      try {
        if(calculator->GetStatistics()->TimeStepExists(i))
        {
          tempHistogram = calculator->GetStatistics()->GetStatisticsForTimeStep(i).m_Histogram;
          this->m_HistogramVector.push_back(tempHistogram);
        }
      } catch (mitk::Exception) {
        MITK_WARN << ":-(";
      }

    }

  }
}
