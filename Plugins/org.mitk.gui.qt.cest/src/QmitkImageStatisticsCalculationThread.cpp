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

#include "QmitkImageStatisticsCalculationThread.h"

//QT headers
#include <QMessageBox>
#include <QApplication>
#include <mitkImageMaskGenerator.h>
#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkIgnorePixelMaskGenerator.h>

QmitkImageStatisticsCalculationThread::QmitkImageStatisticsCalculationThread():QThread(),
  m_StatisticsImage(nullptr), m_BinaryMask(nullptr), m_PlanarFigureMask(nullptr), m_TimeStep(0),
  m_IgnoreZeros(false), m_CalculationSuccessful(false), m_StatisticChanged(false), m_HistogramBinSize(10.0), m_UseDefaultNBins(true), m_nBinsForHistogramStatistics(100), m_prioritizeNBinsOverBinSize(true)
{
}

QmitkImageStatisticsCalculationThread::~QmitkImageStatisticsCalculationThread()
{
}

void QmitkImageStatisticsCalculationThread::Initialize( mitk::Image::Pointer image, mitk::Image::Pointer binaryImage, mitk::PlanarFigure::Pointer planarFig )
{
  // reset old values
  if( this->m_StatisticsImage.IsNotNull() )
    this->m_StatisticsImage = nullptr;

  if( this->m_BinaryMask.IsNotNull() )
    this->m_BinaryMask = nullptr;

  if( this->m_PlanarFigureMask.IsNotNull())
    this->m_PlanarFigureMask = nullptr;

  // set new values if passed in
  if(image.IsNotNull())
    this->m_StatisticsImage = image->Clone();
  if(binaryImage.IsNotNull())
    this->m_BinaryMask = binaryImage->Clone();
  if(planarFig.IsNotNull())
    this->m_PlanarFigureMask = planarFig->Clone();
}

void QmitkImageStatisticsCalculationThread::SetUseDefaultNBins(bool useDefault)
{
    m_UseDefaultNBins = useDefault;
}

void QmitkImageStatisticsCalculationThread::SetTimeStep( int times )
{
  this->m_TimeStep = times;
}

int QmitkImageStatisticsCalculationThread::GetTimeStep()
{
  return this->m_TimeStep;
}

std::vector<mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer> QmitkImageStatisticsCalculationThread::GetStatisticsData()
{
  return this->m_StatisticsVector;
}

mitk::Image::Pointer QmitkImageStatisticsCalculationThread::GetStatisticsImage()
{
  return this->m_StatisticsImage;
}

void QmitkImageStatisticsCalculationThread::SetIgnoreZeroValueVoxel(bool _arg)
{
  this->m_IgnoreZeros = _arg;
}

bool QmitkImageStatisticsCalculationThread::GetIgnoreZeroValueVoxel()
{
  return this->m_IgnoreZeros;
}

void QmitkImageStatisticsCalculationThread::SetHistogramBinSize(double size)
{
  this->m_HistogramBinSize = size;
  this->m_prioritizeNBinsOverBinSize = false;
}

double QmitkImageStatisticsCalculationThread::GetHistogramBinSize() const
{
  return this->m_HistogramBinSize;
}

void QmitkImageStatisticsCalculationThread::SetHistogramNBins(double size)
{
  this->m_nBinsForHistogramStatistics = size;
  this->m_prioritizeNBinsOverBinSize = true;
}

double QmitkImageStatisticsCalculationThread::GetHistogramNBins() const
{
  return this->m_nBinsForHistogramStatistics;
}

std::string QmitkImageStatisticsCalculationThread::GetLastErrorMessage()
{
  return m_message;
}

QmitkImageStatisticsCalculationThread::HistogramType::Pointer
QmitkImageStatisticsCalculationThread::GetTimeStepHistogram(unsigned int t)
{
  if (t >= this->m_HistogramVector.size())
    return nullptr;

  return this->m_HistogramVector[t];
}

bool QmitkImageStatisticsCalculationThread::GetStatisticsChangedFlag()
{
  return m_StatisticChanged;
}

bool QmitkImageStatisticsCalculationThread::GetStatisticsUpdateSuccessFlag()
{
  return m_CalculationSuccessful;
}

void QmitkImageStatisticsCalculationThread::run()
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
      imgMask->SetImageMask(m_BinaryMask);
      calculator->SetMask(imgMask.GetPointer());
    }
    if(this->m_PlanarFigureMask.IsNotNull())
    {
      mitk::PlanarFigureMaskGenerator::Pointer pfMaskGen = mitk::PlanarFigureMaskGenerator::New();
      pfMaskGen->SetInputImage(m_StatisticsImage);
      pfMaskGen->SetPlanarFigure(m_PlanarFigureMask);
      calculator->SetMask(pfMaskGen.GetPointer());
    }
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_ERROR << "ITK Exception:" << e.what();
    statisticCalculationSuccessful = false;
  }
  catch( const mitk::Exception& e )
  {
    MITK_ERROR<< "MITK Exception: " << e.what();
    statisticCalculationSuccessful = false;
  }
  catch ( const std::runtime_error &e )
  {
    MITK_ERROR<< "Runtime Exception: " << e.what();
    statisticCalculationSuccessful = false;
  }
  catch ( const std::exception &e )
  {
    //m_message = "Failure: " + std::string(e.what());
    MITK_ERROR<< "Standard Exception: " << e.what();
    statisticCalculationSuccessful = false;
  }

  bool statisticChanged = false;

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

  if (m_UseDefaultNBins)
  {
      calculator->SetNBinsForHistogramStatistics(100);
  }
  else
  {
      if (!m_prioritizeNBinsOverBinSize)
      {
          calculator->SetBinSizeForHistogramStatistics(m_HistogramBinSize);
      }
      else
      {
          calculator->SetNBinsForHistogramStatistics(100);
      }
  }

  //calculator->SetHistogramBinSize( m_HistogramBinSize );
  //calculator->SetUseDefaultBinSize( m_UseDefaultBinSize );

  for (unsigned int i = 0; i < m_StatisticsImage->GetTimeSteps(); i++)
  {
    try
    {
      calculator->GetStatistics(i);
    }
    catch ( mitk::Exception& e)
    {
      //m_message = e.GetDescription();
      MITK_ERROR<< "MITK Exception: " << e.what();
      statisticCalculationSuccessful = false;
    }
    catch ( const std::runtime_error &e )
    {
      //m_message = "Failure: " + std::string(e.what());
      MITK_ERROR<< "Runtime Exception: " << e.what();
      statisticCalculationSuccessful = false;
    }
    catch ( const std::exception &e )
    {
      //m_message = "Failure: " + std::string(e.what());
      MITK_ERROR<< "Standard Exception: " << e.what();
      statisticCalculationSuccessful = false;
    }
  }

  this->m_StatisticChanged = statisticChanged;
  this->m_CalculationSuccessful = statisticCalculationSuccessful;

  if(statisticCalculationSuccessful)
  {
    this->m_StatisticsVector.clear();
    this->m_HistogramVector.clear();

    for (unsigned int i = 0; i < m_StatisticsImage->GetTimeSteps(); i++)
    {
      this->m_StatisticsVector.push_back(calculator->GetStatistics(i));
      this->m_HistogramVector.push_back((HistogramType*)this->m_StatisticsVector[i]->GetHistogram());
    }
  }
}
