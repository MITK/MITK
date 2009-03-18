/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkHistogramValuesWidget.h"

#include <qlabel.h>
#include <qpen.h>
#include <qvgroupbox.h>

#include <qwt_plot_grid.h>
#include <qwt_interval_data.h>
#include <qwt_array.h>
#include <qwt_text_label.h>
#include <qwt_text.h>
#include <qwt_symbol.h>

//#include <iostream>

QmitkHistogramValuesWidget::QmitkHistogramValuesWidget(QWidget *parent, bool showreport)
{
  QBoxLayout *layout = new QVBoxLayout(this);

  //***histogram***

  QVGroupBox *hgroupbox = new QVGroupBox("", this, "histogram");

  hgroupbox->setMinimumSize(900, 400);

  m_Plot = new QwtPlot(hgroupbox);
  m_Plot->setCanvasBackground(QColor(Qt::white));
  m_Plot->setTitle("Histogram");
  QwtText text = m_Plot->titleLabel()->text();
  text.setFont(QFont("Helvetica", 12, QFont::Normal));

  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->enableYMin(true);
  grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
  grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
  grid->attach(m_Plot);

  layout->addWidget(hgroupbox);

  layout->setSpacing(20);

  if (showreport == true)
  {
    //***report***
    QVGroupBox *rgroupbox = new QVGroupBox("", this, "report");

    rgroupbox->setMinimumSize(900, 400);

    QLabel *label = new QLabel("Gray  Value  Analysis", rgroupbox, "report");
    label->setAlignment(AlignHCenter);
    label->setFont(QFont("Helvetica", 14, QFont::Bold));

    m_Textedit = new QTextEdit(rgroupbox);
    m_Textedit->setFont(QFont("Helvetica", 12, QFont::Normal));
    m_Textedit->setReadOnly(true);

    layout->addWidget(rgroupbox);
  }

  m_Picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
    QwtPicker::PointSelection, 
    QwtPlotPicker::NoRubberBand, QwtPicker::AlwaysOn, 
    m_Plot->canvas());

  connect(m_Picker, SIGNAL(selected(const QwtDoublePoint &)),
    SLOT(OnSelect(const QwtDoublePoint &)));
}

QmitkHistogramValuesWidget::~QmitkHistogramValuesWidget()
{

}

void QmitkHistogramValuesWidget::SetHistogram(HistogramType::ConstPointer itkHistogram)
{
  HistogramType::SizeType size = itkHistogram->GetSize();
  HistogramType::IndexType index;
  HistogramType::MeasurementVectorType currentMeasurementVector;

  QwtArray<QwtDoubleInterval> xValues(size[0]);
  QwtArray<double> yValues(size[0]);

  for (size_t i = 0; i < size[0]; ++i)
  {
    index[0] = static_cast<HistogramType::IndexType::IndexValueType> (i);
    currentMeasurementVector = itkHistogram->GetMeasurementVector(index);
    if (currentMeasurementVector[0] != 0.0)
    {
      xValues[i] = QwtDoubleInterval(Round(currentMeasurementVector[0]-1), Round(currentMeasurementVector[0]));
      yValues[i] = static_cast<double> (itkHistogram->GetFrequency(index));
    }
  }

  // rebuild the plot
  m_Plot->clear();

  m_Histogram = new QmitkHistogram();
  m_Histogram->setColor(Qt::darkCyan);
  m_Histogram->setData(QwtIntervalData(xValues, yValues));
  m_Histogram->attach(m_Plot);

  this->InitializeMarker();
  this->InitializeZoomer();

  m_Plot->replot();
}

void QmitkHistogramValuesWidget::SetHistogram( mitk::Image* mitkImage )
{
  this->SetHistogram(mitkImage->GetScalarHistogram());
}

void QmitkHistogramValuesWidget::SetReport(std::string report)
{
  m_Textedit->setText(report.c_str());
}

void QmitkHistogramValuesWidget::InitializeMarker()
{
  m_Marker = new QwtPlotMarker();
  m_Marker->setXValue(0.);
  m_Marker->setLineStyle(QwtPlotMarker::VLine);
  m_Marker->setLabelAlignment(Qt::AlignHCenter | Qt::AlignRight);
  m_Marker->setLinePen(QPen(QColor(200,150,0), 3, Qt::SolidLine));
  m_Marker->setSymbol( QwtSymbol(QwtSymbol::Diamond, 
    QColor(Qt::red), QColor(Qt::red), QSize(10,10)));
  m_Marker->attach(m_Plot);
}


void QmitkHistogramValuesWidget::InitializeZoomer()
{
  m_Zoomer = new QwtPlotZoomer(m_Plot->xBottom, m_Plot->yLeft, m_Plot->canvas());
  m_Zoomer->setRubberBandPen(QPen(Qt::red, 2, Qt::DotLine));
  m_Zoomer->setTrackerPen(QPen(Qt::red));
  m_Zoomer->setSelectionFlags(QwtPlotZoomer::RectSelection);
}

void QmitkHistogramValuesWidget::OnSelect( const QwtDoublePoint& pos )
{
  m_Marker->setXValue( this->Round(pos.x()) );
  //unsigned int count = (unsigned int)(m_Histogram->data().value(pos.x()));
  QString str = QString( "%1" )
    .arg( (int)(this->Round(pos.x())), 0, 10 );
  QwtText text(str);
  text.setBackgroundBrush(QColor(200,150,0));
  text.setFont(QFont("Helvetica", 14, QFont::Bold));
  m_Marker->setLabel(text);
  m_Plot->replot();
}

double QmitkHistogramValuesWidget::GetMarkerPosition()
{
  return m_Marker->xValue();
}

double QmitkHistogramValuesWidget::Round(double val)
{
  double ival = (double)(int)val;
  if( (val - ival) > 0.5)
    return ival+1;
  else
    return ival;
}
