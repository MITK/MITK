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
#include <vtkQtChartLegend.h>
#include <vtkQtChartLegendManager.h>

//#include <iostream>

QmitkVtkHistogramWidget::QmitkVtkHistogramWidget( QWidget * parent )
: QDialog(parent), m_HistogramMode( HISTOGRAM_MODE_ENTIREIMAGE )
{
  //QGroupBox *hgroupbox = new QGroupBox( "", this );
  //hgroupbox->setMinimumSize( 150, 150 );

  m_ChartWidget = new vtkQtChartWidget( this );

  QBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( m_ChartWidget );
  layout->setSpacing( 10 );

  vtkQtChartArea *area = m_ChartWidget->getChartArea();

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
}



QmitkVtkHistogramWidget::~QmitkVtkHistogramWidget()
{
}


void QmitkVtkHistogramWidget::SetHistogramModeToDirectHistogram()
{
  if ( m_HistogramMode != HISTOGRAM_MODE_DIRECT )
  {
    m_HistogramMode = HISTOGRAM_MODE_DIRECT;
  }
}


void QmitkVtkHistogramWidget::SetHistogramModeToEntireImage()
{
  if ( m_HistogramMode != HISTOGRAM_MODE_ENTIREIMAGE )
  {
    m_HistogramMode = HISTOGRAM_MODE_ENTIREIMAGE;
  }
}


void QmitkVtkHistogramWidget::SetHistogramModeToMaskedImage()
{
  if ( m_HistogramMode != HISTOGRAM_MODE_MASKEDIMAGE )
  {
    m_HistogramMode = HISTOGRAM_MODE_MASKEDIMAGE;
  }
}


void QmitkVtkHistogramWidget::SetHistogramModeToImageRegion()
{
  if ( m_HistogramMode != HISTOGRAM_MODE_IMAGEREGION )
  {
    m_HistogramMode = HISTOGRAM_MODE_IMAGEREGION;
  }
}

void QmitkVtkHistogramWidget::SetHistogramModeToPlanarFigureRegion()
{
  if ( m_HistogramMode != HISTOGRAM_MODE_PLANARFIGUREREGION )
  {
    m_HistogramMode = HISTOGRAM_MODE_PLANARFIGUREREGION;
  }
}


void QmitkVtkHistogramWidget::UpdateItemModelFromHistogram()
{
  this->ComputeHistogram();

  if ( m_DerivedHistogram.IsNull() )
  {
    return;
  }

  // Determine non-zero range of histogram
  unsigned int startIndex = 0, endIndex = 0, i = 0;
  HistogramConstIteratorType startIt = m_DerivedHistogram->End();
  HistogramConstIteratorType endIt = m_DerivedHistogram->End();
  bool firstNonEmptyBinFound = false;
  for (HistogramConstIteratorType it = m_DerivedHistogram->Begin(); it != m_DerivedHistogram->End(); ++it, ++i )
  {
    if ( it.GetFrequency() > 0.0 )
    {
      endIt = it;
      endIndex = i;
      if ( !firstNonEmptyBinFound )
      {
        firstNonEmptyBinFound = true;
        startIt = it;
        startIndex = i;
      }
    }
  }
  ++endIt;

  // For empty image / mask: clear histogram
  if ( startIt == m_DerivedHistogram->End() )
  {
    this->ClearItemModel();
    return;
  }

  // Allocate data in item model
  m_ItemModel->setRowCount( endIndex + 1 - startIndex );
  m_ItemModel->setColumnCount( 1 );

  // Fill item model with histogram data
  i = 0;
  for (HistogramConstIteratorType it = startIt; it != endIt; ++it)
  {
    const double &frequency = it.GetFrequency();
    const double &measurement = it.GetMeasurementVector()[0];

    m_ItemModel->setVerticalHeaderItem( i, new QStandardItem() );
    m_ItemModel->verticalHeaderItem( i )->setData(
      QVariant( measurement ), Qt::DisplayRole );

    m_ItemModel->setItem( i, 0, new QStandardItem() );
    m_ItemModel->item( i, 0 )->setData( QVariant( frequency ), Qt::DisplayRole );

    ++i;
  }

  vtkQtChartTableSeriesModel *table =
    new vtkQtChartTableSeriesModel( m_ItemModel, m_BarChart );
  m_BarChart->setModel( table );

  m_ChartWidget->show();
}


void QmitkVtkHistogramWidget::ClearItemModel()
{
  m_ItemModel->clear();
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

void QmitkVtkHistogramWidget::SetHistogram(const HistogramType* histogram )
{
  m_Histogram = histogram;
}

void QmitkVtkHistogramWidget::SetImage(const mitk::Image* image )
{
  m_Image = image;
}

void QmitkVtkHistogramWidget::SetImageMask(const mitk::Image* imageMask )
{
  m_ImageMask = imageMask;
}

void QmitkVtkHistogramWidget::SetImageRegion( const RegionType imageRegion )
{
  m_ImageRegion = imageRegion;
}

void QmitkVtkHistogramWidget::SetPlanarFigure(const mitk::PlanarFigure* planarFigure )
{
  m_PlanarFigure = planarFigure;
}

void QmitkVtkHistogramWidget::SetHistogramMode( unsigned int histogramMode )
{
  m_HistogramMode = histogramMode;
}

unsigned int QmitkVtkHistogramWidget::GetHistogramMode()
{
  return m_HistogramMode;
}
