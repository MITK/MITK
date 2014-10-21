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

#include "QmitkIVIMWidget.h"

#include "mitkHistogramGenerator.h"


#include <qwt_scale_engine.h>
#include <qwt_legend.h>


QmitkIVIMWidget::QmitkIVIMWidget( QWidget * parent )
  : QmitkPlotWidget(parent)
{
  //  this->SetAxisTitle( QwtPlot::xBottom, "Grayvalue" );
  //  this->SetAxisTitle( QwtPlot::yLeft, "Probability" );
  //  this->Replot();
  QFrame* canvas = qobject_cast<QFrame*>(m_Plot->canvas());
  if (canvas)
  {
    canvas->setLineWidth(0);
    canvas->setContentsMargins(0,0,0,0);
  }

  QwtLogScaleEngine* logScale = new QwtLogScaleEngine();
  m_Plot->setAxisScaleEngine(0, logScale);

  m_Plot->setAxisScale( 0, 0.15, 1.0 );
}



QmitkIVIMWidget::~QmitkIVIMWidget()
{
}


void QmitkIVIMWidget::DrawGauss()
{

}


void QmitkIVIMWidget::ClearItemModel()
{

}

std::vector<double> QmitkIVIMWidget::vec(const vnl_vector<double>& vector)
{
  std::vector<double> retval(vector.size());
  for(unsigned int i=0; i<vector.size(); i++)
  {
    retval.at(i) = vector[i];
  }
  return retval;
}

void QmitkIVIMWidget::SetParameters( IVIMFilterType::IVIMSnapshot snap )
{
  this->Clear();
  if (snap.bvalues.empty())
      return;

  QString s("f=%1, D=%2, D*=%3");
  s = s.arg(snap.currentF,4);
  s = s.arg(snap.currentD,4);
  s = s.arg(snap.currentDStar,4);
  int curveId = this->InsertCurve( s.toLatin1() );
  this->SetCurvePen( curveId, QPen( Qt::NoPen ) );

  curveId = this->InsertCurve( "ignored measurement points" );
  this->SetCurveData( curveId, vec(snap.bvalues), vec(snap.allmeas) );
  this->SetCurvePen( curveId, QPen(Qt::NoPen) );
  QwtSymbol* whiteSymbol = new QwtSymbol(QwtSymbol::Diamond, QColor(Qt::white), QColor(Qt::black), QSize(10,10));
  this->SetCurveSymbol(curveId, whiteSymbol);

  if(snap.currentDStar != 0)
  {
    curveId = this->InsertCurve( "additional points second fit" );
    this->SetCurveData( curveId, vec(snap.bvals2), vec(snap.meas2) );
    this->SetCurvePen( curveId, QPen( Qt::NoPen ) );
    QwtSymbol* blackSymbol = new QwtSymbol(QwtSymbol::Diamond, QColor(Qt::black), QColor(Qt::black), QSize(10,10));
    this->SetCurveSymbol(curveId, blackSymbol);
  }

  curveId = this->InsertCurve( "points first fit" );
  this->SetCurveData( curveId, vec(snap.bvals1), vec(snap.meas1) );
  this->SetCurvePen( curveId, QPen( Qt::NoPen ) );
  QwtSymbol* redSymbol = new QwtSymbol(QwtSymbol::Diamond, QColor(Qt::red), QColor(Qt::red), QSize(10,10));
  this->SetCurveSymbol(curveId, redSymbol);

  QPen pen;
  pen.setColor( QColor(Qt::red) );
  pen.setWidth(2);
  double maxb = snap.bvalues.max_value();
  vnl_vector<double> xvals(2);
  vnl_vector<double> yvals(2);
  xvals[0] = 0;
  xvals[1] = maxb;
  yvals[0] = 1-snap.currentFunceiled;
  yvals[1] = yvals[0]*exp(-maxb * snap.currentD);
  curveId = this->InsertCurve( "contribution of D to the signal" );
  this->SetCurveData( curveId, vec(xvals), vec(yvals) );
  this->SetCurvePen( curveId, pen );

  if(snap.currentDStar != 0)
  {
    pen.setColor(Qt::black);
    int nsampling = 50;
    xvals.set_size(nsampling);
    yvals.set_size(nsampling);
    double f = 1-snap.currentFunceiled;
    for(int i=0; i<nsampling; i++)
    {
      xvals[i] = (((1.0)*i)/(1.0*nsampling))*maxb;
      yvals[i] = f*exp(- xvals[i] * snap.currentD) + (1-f)*exp(- xvals[i] * (snap.currentD+snap.currentDStar));
    }
    curveId = this->InsertCurve( "resulting fit of the model" );
    this->SetCurveData( curveId, vec(xvals), vec(yvals) );
    this->SetCurvePen( curveId, pen );
  }

//  QMargins margins;
//  margins.setBottom(0);
//  margins.setLeft(0);
//  margins.setRight(0);
//  margins.setTop(0);

  QwtLegend* legend = new QwtLegend();
//  legend->setContentsMargins(margins);
  m_Plot->insertLegend(legend, QwtPlot::BottomLegend);

  this->Replot();

}
