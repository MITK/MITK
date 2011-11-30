/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkODFDetailsWidget.h"

#include <qwt_scale_engine.h>

QmitkODFDetailsWidget::QmitkODFDetailsWidget( QWidget * parent )
  : QmitkPlotWidget(parent)
{
  m_Plot->setCanvasLineWidth(0);
  m_Plot->setMargin(0);

  QwtLinearScaleEngine* scale = new QwtLinearScaleEngine();
  m_Plot->setAxisScaleEngine(0, scale);

  m_Plot->setAxisScale   ( 0, -0.5, 0.5 );
}

QmitkODFDetailsWidget::~QmitkODFDetailsWidget()
{
}

void QmitkODFDetailsWidget::SetParameters( itk::OrientationDistributionFunction<double, QBALL_ODFSIZE> odf )
{
  this->Clear();

  std::vector<double> xVals;
  std::vector<double> yVals;
  float max = itk::NumericTraits<float>::min();
  float min = itk::NumericTraits<float>::max();

  for (int i=0; i<QBALL_ODFSIZE; i++){
    xVals.push_back(i);
    yVals.push_back(odf[i]);
    if (odf[i]>max)
      max = odf[i];
    if (odf[i]<min)
      min = odf[i];
  }

  if(min>0)
    m_Plot->setAxisScale ( 0, 0, max );
  else
    m_Plot->setAxisScale ( 0, min, max );

  int curveId = this->InsertCurve( "ODF Values" );
  this->SetCurveData( curveId, xVals, yVals );
  this->SetCurvePen( curveId, QPen(Qt::blue, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );

  QwtLegend* legend = new QwtLegend();
  //m_Plot->insertLegend(legend, QwtPlot::BottomLegend);

  this->Replot();
}

