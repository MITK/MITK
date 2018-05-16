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

#include "QmitkPartialVolumeAnalysisWidget.h"

#include <QPen>
#include <QFrame>

QmitkPartialVolumeAnalysisWidget::QmitkPartialVolumeAnalysisWidget( QWidget * parent )
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
}



QmitkPartialVolumeAnalysisWidget::~QmitkPartialVolumeAnalysisWidget()
{
}


void QmitkPartialVolumeAnalysisWidget::DrawGauss()
{

}


void QmitkPartialVolumeAnalysisWidget::ClearItemModel()
{

}

void QmitkPartialVolumeAnalysisWidget::SetParameters( ParamsType *params, ResultsType *results, HistType *hist )
{
  this->Clear();

  if(params != nullptr && results != nullptr)
  {
    for(unsigned int i=0; i<m_Vals.size(); i++)
    {
      delete m_Vals[i];
    }

    m_Vals.clear();
    m_Vals.push_back(hist->GetXVals());
    m_Vals.push_back(hist->GetHVals());


    std::vector<double> *xVals = hist->GetXVals();
    std::vector<double> *fiberVals = new std::vector<double>(results->GetFiberVals());
    std::vector<double> *nonFiberVals = new std::vector<double>(results->GetNonFiberVals());
    std::vector<double> *mixedVals = new std::vector<double>(results->GetMixedVals());
    std::vector<double> *combiVals = new std::vector<double>(results->GetCombiVals());


    double fiberFA = 0.0;
    double weights = 0.0;

    for(unsigned int i=0; i<xVals->size(); ++i)
    {

      fiberFA += xVals->at(i) * fiberVals->at(i);
      weights += fiberVals->at(i);
    }

    fiberFA = fiberFA / weights;

    QPen pen( Qt::SolidLine );
    pen.setWidth(1);

    QwtText plot_title("Compartment Histograms " );
    plot_title.setFont( QFont("Helvetica", 10, QFont::Bold) );
    this->SetPlotTitle( plot_title );

    pen.setColor(Qt::black);
    int curveId = this->InsertCurve( "Histogram" );
    this->SetCurveData( curveId, (*m_Vals[0]), (*m_Vals[1]) );
    this->SetCurvePen( curveId, pen );
    this->SetCurveAntialiasingOn( curveId );
    //  this->SetCurveTitle( curveId, "Image Histogram" );

    pen.setColor(Qt::blue);
    pen.setWidth(2);
    curveId = this->InsertCurve( "Combined" );
    this->SetCurveData( curveId, (*hist->GetXVals()), (*combiVals) );
    this->SetCurvePen( curveId, pen );
    m_Vals.push_back(combiVals);
    this->SetCurveAntialiasingOn( curveId );


    curveId = this->InsertCurve( "Tissue class" );
    this->SetCurveData(curveId, (*hist->GetXVals()), (*fiberVals));
    this->SetCurvePen( curveId, QPen( Qt::NoPen ) );
    this->SetCurveBrush(curveId, QBrush(QColor::fromRgbF(1,0,0,.5), Qt::SolidPattern));
    m_Vals.push_back(fiberVals);


    curveId = this->InsertCurve( "Non-tissue class" );
    this->SetCurveData( curveId, (*hist->GetXVals()), (*nonFiberVals) );
    this->SetCurvePen( curveId, QPen( Qt::NoPen ) );
    this->SetCurveBrush(curveId, QBrush(QColor::fromRgbF(0,1,0,.5), Qt::SolidPattern));
    m_Vals.push_back(nonFiberVals);


    curveId = this->InsertCurve( "Mixed (PV) class" );
    this->SetCurveData( curveId, (*hist->GetXVals()), (*mixedVals) );
    this->SetCurvePen( curveId, QPen( Qt::NoPen ) );
    this->SetCurveBrush(curveId, QBrush(QColor::fromRgbF(.7,.7,.7,.5), Qt::SolidPattern));
    m_Vals.push_back(mixedVals);

    auto legend = new QwtLegend();
    m_Plot->insertLegend( legend, QwtPlot::TopLegend );

    this->Replot();



  }

  this->Replot();

}

