/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2009-10-18 21:46:13 +0200 (So, 18 Okt 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkVtkHistogramWidget.h"

#include "mitkHistogramGenerator.h"

#include <qlabel.h>
#include <qpen.h>
#include <qgroupbox.h>


#include <vtkQtChartArea.h>
#include <vtkQtChartTableSeriesModel.h>
#include <vtkQtChartStyleManager.h>
#include <vtkQtChartColorStyleGenerator.h>

#include <vtkQtChartMouseSelection.h>
#include <vtkQtChartInteractorSetup.h>
#include <vtkQtChartSeriesSelectionHandler.h>
#include <vtkQtChartAxisLayer.h>
#include <vtkQtChartAxis.h>
#include <vtkQtChartAxisOptions.h>


//#include <iostream>

QmitkVtkHistogramWidget::QmitkVtkHistogramWidget( QWidget *parent )
: m_HistogramMode( HISTOGRAM_MODE_ENTIREIMAGE )
{
  //QGroupBox *hgroupbox = new QGroupBox( "", this );
  //hgroupbox->setMinimumSize( 150, 150 );

  m_ChartWidget = new vtkQtChartWidget( this );

  QBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( m_ChartWidget );
  layout->setSpacing( 10 );


  //m_BarChart = new vtkBarChart



  vtkQtChartArea *area = m_ChartWidget->getChartArea();
  vtkQtChartStyleManager *style = area->getStyleManager();
  vtkQtChartColorStyleGenerator *generator =
    qobject_cast<vtkQtChartColorStyleGenerator *>( style->getGenerator() );
  if ( generator )
  {
    generator->getColors()->setColorScheme( vtkQtChartColors::Blues );
  }
  else
  {
    style->setGenerator(
      new vtkQtChartColorStyleGenerator( m_ChartWidget, vtkQtChartColors::Blues ) );
  }

  // Set up the bar chart.
  m_BarChart = new vtkQtBarChart();
  area->insertLayer( area->getAxisLayerIndex(), m_BarChart );

  // Set up the default interactor.
  vtkQtChartMouseSelection *selector =
    vtkQtChartInteractorSetup::createDefault( area );
  vtkQtChartSeriesSelectionHandler *handler =
    new vtkQtChartSeriesSelectionHandler( selector );
  handler->setModeNames( "Bar Chart - Series", "Bar Chart - Bars" );
  handler->setMousePressModifiers( Qt::ControlModifier, Qt::ControlModifier );
  handler->setLayer( m_BarChart );
  selector->addHandler( handler );
  selector->setSelectionMode("Bar Chart - Bars");

  // Hide the x-axis grid.
  vtkQtChartAxisLayer *axisLayer = area->getAxisLayer();
  vtkQtChartAxis *xAxis = axisLayer->getAxis(vtkQtChartAxis::Bottom);
  xAxis->getOptions()->setGridVisible(false);
  xAxis->getOptions()->setPrecision( 0 );
  xAxis->getOptions()->setNotation( vtkQtChartAxisOptions::Standard );
  
  vtkQtChartAxis *yAxis = axisLayer->getAxis(vtkQtChartAxis::Left);
  yAxis->getOptions()->setPrecision( 0 );
  yAxis->getOptions()->setNotation( vtkQtChartAxisOptions::Standard );

  // Set up the model for the bar chart.
  m_ItemModel = new QStandardItemModel( m_BarChart );
  m_ItemModel->setItemPrototype( new QStandardItem() );
  m_ItemModel->setHorizontalHeaderItem( 0, new QStandardItem("Histogram") );
}



QmitkVtkHistogramWidget::~QmitkVtkHistogramWidget()
{
}


void QmitkVtkHistogramWidget::SetHistogramModeToDirectHistogram()
{
  if ( m_HistogramMode != HISTOGRAM_MODE_DIRECT )
  {
    m_HistogramMode = HISTOGRAM_MODE_DIRECT;
    this->Modified();
  }
}


void QmitkVtkHistogramWidget::SetHistogramModeToEntireImage()
{
  if ( m_HistogramMode != HISTOGRAM_MODE_ENTIREIMAGE )
  {
    m_HistogramMode = HISTOGRAM_MODE_ENTIREIMAGE;
    this->Modified();
  }
}


void QmitkVtkHistogramWidget::SetHistogramModeToMaskedImage()
{
  if ( m_HistogramMode != HISTOGRAM_MODE_MASKEDIMAGE )
  {
    m_HistogramMode = HISTOGRAM_MODE_MASKEDIMAGE;
    this->Modified();
  }
}


void QmitkVtkHistogramWidget::SetHistogramModeToImageRegion()
{
  if ( m_HistogramMode != HISTOGRAM_MODE_IMAGEREGION )
  {
    m_HistogramMode = HISTOGRAM_MODE_IMAGEREGION;
    this->Modified();
  }
}

void QmitkVtkHistogramWidget::SetHistogramModeToPlanarFigureRegion()
{
  if ( m_HistogramMode != HISTOGRAM_MODE_PLANARFIGUREREGION )
  {
    m_HistogramMode = HISTOGRAM_MODE_PLANARFIGUREREGION;
    this->Modified();
  }
}


void QmitkVtkHistogramWidget::UpdateItemModelFromHistogram()
{
  this->ComputeHistogram();

  if ( m_DerivedHistogram.IsNull() )
  {
    return;
  }

  m_ItemModel->setRowCount( m_DerivedHistogram->Size() );

  HistogramConstIteratorType it;
  unsigned int i;
  for ( it = m_DerivedHistogram->Begin(), i = 0;
        it != m_DerivedHistogram->End();
        ++it, ++i )
  {
    const float &frequency = it.GetFrequency();
    const double &measurement = it.GetMeasurementVector()[0];
    
    m_ItemModel->setVerticalHeaderItem( i, new QStandardItem() );
    m_ItemModel->verticalHeaderItem( i )->setData(
      QVariant( measurement ), Qt::DisplayRole );

    m_ItemModel->setItem( i, 0, new QStandardItem() );
    m_ItemModel->item( i, 0 )->setData( frequency, Qt::DisplayRole );
  }

  vtkQtChartTableSeriesModel *table =
    new vtkQtChartTableSeriesModel( m_ItemModel, m_BarChart );
  m_BarChart->setModel( table );
}


void QmitkVtkHistogramWidget::ClearItemModel()
{
  m_ItemModel->setRowCount( 0 );
  vtkQtChartTableSeriesModel *table =
    new vtkQtChartTableSeriesModel( m_ItemModel, m_BarChart );
  m_BarChart->setModel( table );
}


void QmitkVtkHistogramWidget::ComputeHistogram()
{
  switch ( m_HistogramMode )
  {
  case HISTOGRAM_MODE_DIRECT:
    {
      m_DerivedHistogram = m_Histogram;
      break;
    }

  case HISTOGRAM_MODE_ENTIREIMAGE:
    {
      mitk::HistogramGenerator::Pointer histogramGenerator = mitk::HistogramGenerator::New();
      histogramGenerator->SetImage( m_Image );
      histogramGenerator->ComputeHistogram();
      m_DerivedHistogram = histogramGenerator->GetHistogram();
      break;
    }

  case HISTOGRAM_MODE_MASKEDIMAGE:
    {
      break;
    }

  case HISTOGRAM_MODE_IMAGEREGION:
    {
      break;
    }

  case HISTOGRAM_MODE_PLANARFIGUREREGION:
    {
      break;
    }
  }
}
