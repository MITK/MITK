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

QmitkImageStatisticsCalculationThread::QmitkImageStatisticsCalculationThread():QThread(),
  m_StatisticsImage(NULL), m_BinaryMask(NULL), m_PlanarFigureMask(NULL), m_TimeStep(0),
  m_IgnoreZeros(false), m_CalculationSuccessful(false), m_StatisticChanged(false), m_HistogramBinSize(1), m_UseDefaultBinSize(true)
{
}

QmitkImageStatisticsCalculationThread::~QmitkImageStatisticsCalculationThread()
{
}

void QmitkImageStatisticsCalculationThread::Initialize( mitk::Image::Pointer image, mitk::Image::Pointer binaryImage, mitk::PlanarFigure::Pointer planarFig )
{
  // reset old values
  if( this->m_StatisticsImage.IsNotNull() )
    this->m_StatisticsImage = 0;

  if( this->m_BinaryMask.IsNotNull() )
    this->m_BinaryMask = 0;

  if( this->m_PlanarFigureMask.IsNotNull())
    this->m_PlanarFigureMask = 0;

  // set new values if passed in
  if(image.IsNotNull())
    this->m_StatisticsImage = image->Clone();
  if(binaryImage.IsNotNull())
    this->m_BinaryMask = binaryImage->Clone();
  if(planarFig.IsNotNull())
    this->m_PlanarFigureMask = planarFig->Clone();
}

void QmitkImageStatisticsCalculationThread::SetUseDefaultBinSize(bool useDefault)
{
    m_UseDefaultBinSize = useDefault;
}

void QmitkImageStatisticsCalculationThread::SetTimeStep( int times )
{
  this->m_TimeStep = times;
}

int QmitkImageStatisticsCalculationThread::GetTimeStep()
{
  return this->m_TimeStep;
}

std::vector<mitk::ImageStatisticsCalculator::Statistics> QmitkImageStatisticsCalculationThread::GetStatisticsData()
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

void QmitkImageStatisticsCalculationThread::SetHistogramBinSize(unsigned int size)
{
  this->m_HistogramBinSize = size;
}

unsigned int QmitkImageStatisticsCalculationThread::GetHistogramBinSize()
{
  return this->m_HistogramBinSize;
}

std::string QmitkImageStatisticsCalculationThread::GetLastErrorMessage()
{
  return m_message;
}

QmitkImageStatisticsCalculationThread::HistogramType::Pointer
QmitkImageStatisticsCalculationThread::GetTimeStepHistogram(unsigned int t)
{
  if (t >= this->m_HistogramVector.size())
    return NULL;

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
    calculator->SetImage(m_StatisticsImage);
    calculator->SetMaskingModeToNone();
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

      calculator->SetImageMask(m_BinaryMask);
      calculator->SetMaskingModeToImage();
    }
    if(this->m_PlanarFigureMask.IsNotNull())
    {
      calculator->SetPlanarFigure(m_PlanarFigureMask);
      calculator->SetMaskingModeToPlanarFigure();
    }
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_ERROR << "ITK Exception:" << e.what();
    statisticCalculationSuccessful = false;
  }
  bool statisticChanged = false;

  calculator->SetDoIgnorePixelValue(this->m_IgnoreZeros);
  calculator->SetIgnorePixelValue(0);
  calculator->SetHistogramBinSize( m_HistogramBinSize );
  calculator->SetUseDefaultBinSize( m_UseDefaultBinSize );

  for (unsigned int i = 0; i < m_StatisticsImage->GetTimeSteps(); i++)
  {
    try
    {
      statisticChanged = calculator->ComputeStatistics(i);
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
      this->m_HistogramVector.push_back((HistogramType*)calculator->GetHistogram(i));
    }
  }

  m_HistogramBinSize = calculator->GetHistogramBinSize();
}
