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

#include "QmitkNetworkHistogramCanvas.h"

#include <itkObject.h>

#include <QPainter>
#include <qwt_text.h>
#include <qwt_plot_grid.h>
#include <qwt_painter.h>
#include <qwt_legend.h>
#include <qwt_plot_marker.h>

QmitkNetworkHistogramCanvas::QmitkNetworkHistogramCanvas(QWidget * parent,
    Qt::WindowFlags f)
    : QmitkPlotWidget(parent)
{
  setEnabled(false);
  setFocusPolicy(Qt::ClickFocus);

}

QmitkNetworkHistogramCanvas::~QmitkNetworkHistogramCanvas()
{
}

void QmitkNetworkHistogramCanvas::DrawProfiles( )
{
  this->Clear();

  if( !(m_Histogram) || !( m_Histogram->IsValid() ))
  {
    return;
  }

  std::vector<double> histogramVector = m_Histogram->GetHistogramVector();

  // create a data vector which contains the points to be drawn
  // to create nice bars, the curve takes four points for each bar
  std::vector< std::pair< double, double > > dataPointsVector;

  std::pair< double, double > leftBottom, leftTop, rightTop, rightBottom;

  // how wide the bar is, the gap to the next one will be 1 - barWidth
  double barWidth( 0.95 );
  const int range = histogramVector.size();

  for(int i=0; i < range ; ++i)
  {
    leftBottom.first = ((double) i) ;
    leftBottom.second = 0.0;
    leftTop.first = ((double) i) ;
    leftTop.second = histogramVector.at( i );
    rightTop.first = ((double) i) + barWidth ;
    rightTop.second = histogramVector.at( i );
    rightBottom.first = ((double) i) + barWidth ;
    rightBottom.second = 0.0;

    dataPointsVector.push_back( leftBottom );
    dataPointsVector.push_back( leftTop );
    dataPointsVector.push_back( rightTop );
    dataPointsVector.push_back( rightBottom );
  }

  this->SetPlotTitle( (m_Histogram->GetSubject()).c_str() );

  QPen pen( Qt::NoPen );
  QBrush brush( Qt::SolidPattern );

  brush.setColor( Qt::blue );

  int curveId = this->InsertCurve( (m_Histogram->GetSubject()).c_str() );
  this->SetCurveData( curveId, dataPointsVector );

  this->SetCurvePen( curveId, pen );
  this->SetCurveBrush( curveId, brush );

  // Axis 0 is the y axis, axis to the x axis
  this->m_Plot->setAxisTitle(0, "n");
  //this->m_Plot->setAxisTitle(2, "");

  this->Replot();

}
