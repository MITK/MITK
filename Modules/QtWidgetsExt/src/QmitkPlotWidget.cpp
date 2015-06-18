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

#include <qlayout.h>
#include <iostream>

#include <qwt_point_data.h>

#include "QmitkPlotWidget.h"

QmitkPlotWidget::QmitkPlotWidget(QWidget* parent, const char* title, const char*, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  auto  boxLayout = new QVBoxLayout(this);
  m_Plot = new QwtPlot( QwtText(title), this ) ;
  m_Plot->setCanvasBackground(Qt::white);
  boxLayout->addWidget( m_Plot );
}

QmitkPlotWidget::~QmitkPlotWidget()
{
  this->Clear();
  delete m_Plot;
}


QwtPlot* QmitkPlotWidget::GetPlot()
{
  return m_Plot;
}

void QmitkPlotWidget::SetLegend(QwtLegend* legend, QwtPlot::LegendPosition pos, double ratio)
{
  m_Plot->insertLegend(legend, pos, ratio);
}

unsigned int QmitkPlotWidget::InsertCurve(const char* title)
{
  QwtPlotCurve* curve = new QwtPlotCurve(QwtText(title));
  m_PlotCurveVector.push_back(curve);
  curve->attach(m_Plot);
  return static_cast<unsigned int> (m_PlotCurveVector.size() - 1);
}

void QmitkPlotWidget::SetPlotTitle(const char* title)
{
  m_Plot->setTitle(title);
}

void QmitkPlotWidget::SetAxisTitle(int axis, const char* title)
{
  m_Plot->setAxisTitle(axis, title);
}


bool QmitkPlotWidget::SetCurveData( unsigned int curveId, const QmitkPlotWidget::DataVector& xValues, const QmitkPlotWidget::DataVector& yValues )
{
  if ( xValues.size() != yValues.size() )
  {
    std::cerr << "Sizes of data arrays don't match." << std::endl;
    return false;
  }
  double* rawDataX = ConvertToRawArray( xValues );
  double* rawDataY = ConvertToRawArray( yValues );
  m_PlotCurveVector[curveId]->setSamples(new QwtPointArrayData(rawDataX, rawDataY, static_cast<int>(xValues.size())));
  delete[] rawDataX;
  delete[] rawDataY;
  return true;
}


bool QmitkPlotWidget::SetCurveData(unsigned int curveId, const XYDataVector& data )
{
  double* rawDataX = ConvertToRawArray( data, 0 );
  double* rawDataY = ConvertToRawArray( data, 1 );
  m_PlotCurveVector[curveId]->setData(new QwtPointArrayData(rawDataX, rawDataY, static_cast<int>(data.size())));
  delete[] rawDataX;
  delete[] rawDataY;
  return true;
}

void QmitkPlotWidget::SetCurvePen( unsigned int curveId, const QPen& pen )
{
  m_PlotCurveVector[curveId]->setPen( pen );
}


void QmitkPlotWidget::SetCurveBrush( unsigned int curveId, const QBrush& brush )
{
  m_PlotCurveVector[curveId]->setBrush( brush );
}


void QmitkPlotWidget::SetCurveTitle( unsigned int, const char* title )
{
  m_Plot->setTitle( title );
}

void QmitkPlotWidget::SetCurveStyle( unsigned int curveId, const QwtPlotCurve::CurveStyle style )
{
  m_PlotCurveVector[curveId]->setStyle(style);
}

void QmitkPlotWidget::SetCurveSymbol( unsigned int curveId, QwtSymbol* symbol )
{
  m_PlotCurveVector[curveId]->setSymbol(symbol);
}

void QmitkPlotWidget::Replot()
{
  m_Plot->replot();
}


void QmitkPlotWidget::Clear()
{
  m_Plot->detachItems();
  m_PlotCurveVector.clear();
  m_PlotCurveVector.resize(0);

}


double* QmitkPlotWidget::ConvertToRawArray( const QmitkPlotWidget::DataVector& values )
{
  auto  raw = new double[ values.size() ];
  for( unsigned int i = 0; i < values.size(); ++i )
    raw[i] = values[i];
  return raw;
}


double* QmitkPlotWidget::ConvertToRawArray( const QmitkPlotWidget::XYDataVector& values, unsigned int component )
{
  auto  raw = new double[ values.size() ];
  for( unsigned int i = 0; i < values.size(); ++i )
  {
    switch (component)
    {
    case (0):
      raw[i] = values[i].first;
      break;
    case (1):
      raw[i] = values[i].second;
      break;
    default:
      std::cout << "Component must be either 0 or 1."<< std::endl;
    }
  }
  return raw;
}

