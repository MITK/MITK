/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <qlayout.h>
#include <iostream>

#include "QmitkPlotWidget.h"

QmitkPlotWidget::QmitkPlotWidget(QWidget* parent, const char* title, const char* name, WFlags f): QWidget(parent, name, f) 
{
  QVBoxLayout* boxLayout = new QVBoxLayout(this);
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


int QmitkPlotWidget::InsertCurve(const char* title)
{
  QwtPlotCurve* curve = new QwtPlotCurve(QwtText(title));
  m_PlotCurveVector.push_back(curve);
  curve->attach(m_Plot);
  return m_PlotCurveVector.size() - 1;
}

void QmitkPlotWidget::SetPlotTitle(const char* title)
{
  m_Plot->setTitle(title);
}

void QmitkPlotWidget::SetAxisTitle(int axis, const char* title)
{
  m_Plot->setAxisTitle(axis, title);
}


bool QmitkPlotWidget::SetCurveData( int curveId, const QmitkPlotWidget::DataVector& xValues, const QmitkPlotWidget::DataVector& yValues )
{
  if ( xValues.size() != yValues.size() )
  {
    std::cerr << "Sizes of data arrays don't match." << std::endl;
    return false;
  }
  double* rawDataX = ConvertToRawArray( xValues );
  double* rawDataY = ConvertToRawArray( yValues );
  m_PlotCurveVector[curveId]->setData( rawDataX, rawDataY, xValues.size() );
  delete[] rawDataX;
  delete[] rawDataY;
  return true;
}


bool QmitkPlotWidget::SetCurveData( int curveId, const QmitkPlotWidget::XYDataVector& data )
{
  double* rawDataX = ConvertToRawArray( data, 0 );
  double* rawDataY = ConvertToRawArray( data, 1 );
  m_PlotCurveVector[curveId]->setData( rawDataX, rawDataY, data.size() );
  delete[] rawDataX;
  delete[] rawDataY;
  return true;
}


void QmitkPlotWidget::SetCurvePen( int curveId, const QPen& pen )
{
  m_PlotCurveVector[curveId]->setPen( pen );
}


void QmitkPlotWidget::SetCurveBrush( int curveId, const QBrush& brush )
{
  m_PlotCurveVector[curveId]->setBrush( brush );
}


void QmitkPlotWidget::SetCurveTitle( int curveId, const char* title )
{
  m_Plot->setTitle( title );
}


void QmitkPlotWidget::Replot()
{
  m_Plot->replot();
}


void QmitkPlotWidget::Clear()
{
  m_PlotCurveVector.clear();
  m_PlotCurveVector.resize(0);
  m_Plot->clear();
}


double* QmitkPlotWidget::ConvertToRawArray( const QmitkPlotWidget::DataVector& values )
{
  double* raw = new double[ values.size() ];
  for( unsigned int i = 0; i < values.size(); ++i )
    raw[i] = values[i];
  return raw;
}


double* QmitkPlotWidget::ConvertToRawArray( const QmitkPlotWidget::XYDataVector& values, unsigned int component )
{
  double* raw = new double[ values.size() ];
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
