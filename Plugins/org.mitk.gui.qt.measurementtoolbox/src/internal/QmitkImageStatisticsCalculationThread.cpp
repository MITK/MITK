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
  m_IgnoreZeros(false), m_CalculationSuccessful(false), m_StatisticChanged(false)
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
    this->m_PlanarFigureMask = dynamic_cast<mitk::PlanarFigure*>(planarFig.GetPointer()); // once clone methods for planar figures are implemented, copy the data here!
}

void QmitkImageStatisticsCalculationThread::SetTimeStep( int times )
{
  this->m_TimeStep = times;
}

int QmitkImageStatisticsCalculationThread::GetTimeStep()
{
  return this->m_TimeStep;
}

mitk::ImageStatisticsCalculator::Statistics QmitkImageStatisticsCalculationThread::GetStatisticsData()
{
  return this->m_StatisticsStruct;
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

std::string QmitkImageStatisticsCalculationThread::GetLastErrorMessage()
{
  return m_message;
}

QmitkImageStatisticsCalculationThread::HistogramType::Pointer
QmitkImageStatisticsCalculationThread::GetTimeStepHistogram()
{
  return this->m_TimeStepHistogram;
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
  try
  {
    statisticChanged = calculator->ComputeStatistics(m_TimeStep);
  }
  catch ( mitk::Exception& e)
  {
    m_message = e.GetDescription();
    statisticCalculationSuccessful = false;
  }
  catch ( const std::runtime_error &e )
  {
    MITK_ERROR<< "Runtime Exception: " << e.what();
    statisticCalculationSuccessful = false;
  }
  catch ( const std::exception &e )
  {
    MITK_ERROR<< "Standard Exception: " << e.what();
    statisticCalculationSuccessful = false;
  }
  this->m_StatisticChanged = statisticChanged;
  this->m_CalculationSuccessful = statisticCalculationSuccessful;

  if(statisticCalculationSuccessful)
  {
    this->m_StatisticsStruct = calculator->GetStatistics(m_TimeStep);

    if(this->m_TimeStepHistogram.IsNotNull())
    {
      this->m_TimeStepHistogram = NULL;
    }
    this->m_TimeStepHistogram = (HistogramType*) calculator->GetHistogram(m_TimeStep);
  }
}
