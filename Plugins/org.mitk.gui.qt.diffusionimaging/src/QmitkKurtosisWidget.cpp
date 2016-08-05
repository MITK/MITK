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

#include "QmitkKurtosisWidget.h"

#include <qwt_scale_engine.h>
#include <qwt_legend.h>

QmitkKurtosisWidget::QmitkKurtosisWidget(QWidget *parent)
  : QmitkPlotWidget(parent)
{
  QFrame* canvas = qobject_cast< QFrame* >( m_Plot->canvas() );
  if( canvas )
  {
    canvas->setLineWidth(0);
    canvas->setContentsMargins(0, 0, 0, 0);
  }

}

QmitkKurtosisWidget::~QmitkKurtosisWidget()
{

}


void QmitkKurtosisWidget::SetData(KurtosisFilterType::KurtosisSnapshot snap)
{
  this->Clear();

  if( snap.bvalues.empty() )
    return;

  double max_y_val = 1.4 * fmax( snap.measurements[0], snap.m_BzeroFit );

  auto logScale = new QwtLogScaleEngine();
  m_Plot->setAxisScaleEngine(0, logScale );
  m_Plot->setAxisScale(0, 0.1, max_y_val );

  QString s("D=%1, K=%2");
  s = s.arg( snap.m_D, 4); s = s.arg( snap.m_K, 4);

  // insert formatted value string to legend (curve without pen)
  int curveId = this->InsertCurve( s.toLatin1(), QColor( Qt::black ) );
  this->SetCurvePen( curveId, QPen( Qt::NoPen ) );

  QPen pen;
  pen.setColor( QColor( Qt::red ));
  pen.setWidth(2);
  pen.setStyle( Qt::PenStyle::DashLine );

  // get the x-axis maximum
  const double max_bvalue = snap.bvalues.max_value();

  //
  // Data-points
  //
  auto measured_values = toStdVec( snap.measurements );
  double y_bzero = measured_values[0];

  if( snap.m_fittedBZero )
  {
    /*auto c_measurements_curve = this->InsertCurve( "Corrected measured values with fitted b=0" );
    this->SetCurveData( c_measurements_curve, toStdVec( snap.fit_bvalues ), toStdVec( snap.fit_measurements / snap.m_BzeroFit ) );
    this->SetCurvePen( c_measurements_curve, QPen(Qt::NoPen) );
    QwtSymbol* whiteDiamond = new QwtSymbol(QwtSymbol::Diamond, QColor(Qt::white), QColor(Qt::black), QSize(8,8));
    this->SetCurveSymbol( c_measurements_curve, whiteDiamond );*/

    std::vector<double> single_bzero;
    single_bzero.push_back(0);

    std::vector<double> fitted_bzero;
    fitted_bzero.push_back( snap.m_BzeroFit );

    auto c_measurements_bzero = this->InsertCurve( "Fitted b=0" );
    this->SetCurveData( c_measurements_bzero, single_bzero, fitted_bzero );
    this->SetCurvePen( c_measurements_bzero, QPen(Qt::NoPen) );
    QwtSymbol* blackDiamond = new QwtSymbol(QwtSymbol::Diamond, QColor(Qt::black), QColor(Qt::black), QSize(8,8));
    this->SetCurveSymbol( c_measurements_bzero, blackDiamond );

    y_bzero = snap.m_BzeroFit;

    MITK_DEBUG("Kurtosis.Widget.Bzero") << "[Fitted] " << snap.m_BzeroFit << ": [Measured]  " << snap.measurements[0];
  }


  auto measurements_curve = this->InsertCurve( "Measured values" );
  this->SetCurveData( measurements_curve, toStdVec( snap.bvalues ), measured_values );
  this->SetCurvePen( measurements_curve, QPen(Qt::NoPen) );
  QwtSymbol* redDiamond = new QwtSymbol(QwtSymbol::Diamond, QColor(Qt::red), QColor(Qt::red), QSize(8,8));
  this->SetCurveSymbol( measurements_curve, redDiamond );

  //
  // Kurtosis - full modelled signal
  //
  pen.setColor( QColor( Qt::black ));
  pen.setStyle( Qt::SolidLine );
  const unsigned int num_samples = 50;

  vnl_vector<double> x_K_model(num_samples);
  vnl_vector<double> y_K_model(num_samples);

  vnl_vector<double> y_D_model(num_samples);

  const double x_tics_offset = max_bvalue / static_cast<double>( num_samples );

  for( unsigned int i=0; i<num_samples; ++i)
  {
     x_K_model[i] = i * x_tics_offset;

     double bval = x_K_model[i];
     y_K_model[i] = y_bzero * exp( -bval * snap.m_D + bval*bval * snap.m_D * snap.m_D * snap.m_K / 6.0 );
     y_D_model[i] = y_bzero * exp( -bval * snap.m_D );
  }

  auto kurtosis_curve = this->InsertCurve( "Resulting fit of the model" );
  this->SetCurveData( kurtosis_curve, toStdVec( x_K_model ), toStdVec( y_K_model ) );
  this->SetCurvePen( kurtosis_curve, pen );
  this->SetCurveAntialiasingOn( kurtosis_curve );

  auto d_curve = this->InsertCurve( "D-part of the fitted model" );
  this->SetCurveData( d_curve, toStdVec( x_K_model ), toStdVec( y_D_model ) );

  pen.setColor( QColor( Qt::red));
  pen.setStyle( Qt::PenStyle::DashLine );
  this->SetCurvePen( d_curve, pen );
  this->SetCurveAntialiasingOn( d_curve );

  //
  // add Legend
  //
  auto legend = new QwtLegend();
  legend->setMaxColumns(3);
  m_Plot->insertLegend( legend, QwtPlot::BottomLegend );

  this->Replot();
}

std::vector<double> QmitkKurtosisWidget::toStdVec(const vnl_vector<double>& vector)
{
  std::vector<double> retval(vector.size());
  for(unsigned int i=0; i<vector.size(); i++)
  {
    retval.at(i) = vector[i];
  }
  return retval;
}

