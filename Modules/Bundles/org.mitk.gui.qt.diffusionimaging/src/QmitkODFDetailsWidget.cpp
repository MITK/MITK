/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkODFDetailsWidget.h"

#include "mitkHistogramGenerator.h"

#include <qlabel.h>
#include <qpen.h>
#include <qgroupbox.h>
//#include <qmargins.h>


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

#include <qwt_scale_engine.h>
#include <qwt_legend.h>

//#include <iostream>

QmitkODFDetailsWidget::QmitkODFDetailsWidget( QWidget * parent )
  : QmitkPlotWidget(parent)
{
  //  this->SetAxisTitle( QwtPlot::xBottom, "Grayvalue" );
  //  this->SetAxisTitle( QwtPlot::yLeft, "Probability" );
  //  this->Replot();
  m_Plot->setCanvasLineWidth(0);
  m_Plot->setMargin(0);

  QwtLinearScaleEngine* logScale = new QwtLinearScaleEngine();
  m_Plot->setAxisScaleEngine(0, logScale);

  m_Plot->setAxisScale   ( 0, -0.5, 0.5 );
}

QmitkODFDetailsWidget::~QmitkODFDetailsWidget()
{
}

std::vector<double> QmitkODFDetailsWidget::vec(vnl_vector<double> vector)
{
  std::vector<double> retval(vector.size());
  for(unsigned int i=0; i<vector.size(); i++)
  {
    retval.at(i) = vector[i];
  }
  return retval;
}

void QmitkODFDetailsWidget::SetParameters( std::vector<double> odfVals )
{
  this->Clear();

  std::vector<double> xVals;
  float max = -1;
  float min = 1;
  for (int i=0; i<odfVals.size(); i++){
    xVals.push_back(i);
    if (odfVals.at(i)>max)
      max = odfVals.at(i);
    if (odfVals.at(i)<min)
      min = odfVals.at(i);
  }
  if(min>0)
    m_Plot->setAxisScale ( 0, 0, max );
  else
    m_Plot->setAxisScale ( 0, min, max );

  int curveId = this->InsertCurve( "ODF Values" );
  this->SetCurveData( curveId, xVals, odfVals );
  this->SetCurvePen( curveId, QPen(Qt::blue, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );

  QwtLegend* legend = new QwtLegend();
  m_Plot->insertLegend(legend, QwtPlot::BottomLegend);

  this->Replot();
}

