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

#include "QmitkResidualAnalysisWidget.h"


#include <qlabel.h>
#include <qpen.h>
#include <qgroupbox.h>

#include <qwt_text.h>
#include <qwt_plot_grid.h>
#include <qwt_painter.h>
#include <qwt_legend.h>
#include <qwt_plot_marker.h>


#include <iostream>
#include <fstream>


QmitkResidualAnalysisWidget::QmitkResidualAnalysisWidget( QWidget * parent )
  : QmitkPlotWidget(parent)
{
  m_PlotPicker = new QwtPlotPicker(m_Plot->canvas());
  m_PlotPicker->setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection | QwtPicker::DragSelection);
  m_PlotPicker->setTrackerMode(QwtPicker::ActiveOnly);
}



QmitkResidualAnalysisWidget::~QmitkResidualAnalysisWidget()
{
  delete m_PlotPicker;
}


void QmitkResidualAnalysisWidget::DrawMeans()
{
  this->Clear();
  this->SetPlotTitle("mean residual per volume");
  QPen pen( Qt::SolidLine );
  pen.setWidth(1);


  // Create values for x-axis
  std::vector<double> xAxis;
  for(int i=0; i<m_Means.size(); ++i)
  {
    xAxis.push_back((double)i);
  }

  pen.setColor(Qt::black);
  int curveId = this->InsertCurve( "Mean" );
  this->SetCurveData( curveId, xAxis, m_Means );
  this->SetCurvePen( curveId, pen );
  this->SetCurveStyle( curveId, QwtPlotCurve::Dots);

  pen.setColor(Qt::blue);
  curveId = this->InsertCurve( "Q1" );
  this->SetCurveData( curveId, xAxis, m_Q1 );
  this->SetCurvePen( curveId, pen );
  this->SetCurveStyle( curveId, QwtPlotCurve::Dots);

  pen.setColor(Qt::red);
  curveId = this->InsertCurve( "Q3" );
  this->SetCurveData( curveId, xAxis, m_Q3 );
  this->SetCurvePen( curveId, pen );
  this->SetCurveStyle( curveId, QwtPlotCurve::Dots);


  this->m_Plot->setAxisTitle(0, "Residual");
  this->m_Plot->setAxisTitle(3, "DWI Volume");


  QwtLegend *legend = new QwtLegend;
  this->SetLegend(legend, QwtPlot::RightLegend, 0.5);

  this->Replot();

}

void QmitkResidualAnalysisWidget::DrawPercentagesOfOutliers()
{
  this->Clear();


  this->SetPlotTitle("Percentage of outliers");
  QPen pen( Qt::SolidLine );
  pen.setWidth(1);


  // Create values for x-axis
  std::vector<double> xAxis;
  for(int i=0; i<m_PercentagesOfOutliers.size(); ++i)
  {
    xAxis.push_back((double)i);
  }

  pen.setColor(Qt::black);
  int curveId = this->InsertCurve( "Outliers" );
  this->SetCurveData( curveId, xAxis, m_PercentagesOfOutliers );
  this->SetCurvePen( curveId, pen );
  //this->SetCurveStyle( curveId, QwtPlotCurve::Fitted);

  this->m_Plot->setAxisTitle(0, "Percentage of outliers");
  this->m_Plot->setAxisTitle(3, "DWI Volume");

  QwtLegend *legend = new QwtLegend;
  this->SetLegend(legend, QwtPlot::RightLegend, 0.5);

  this->Replot();
}




