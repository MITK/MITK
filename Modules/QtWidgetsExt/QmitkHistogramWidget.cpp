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

#include "QmitkHistogramWidget.h"

#include <QmitkHistogram.h>

#include "mitkImageStatisticsHolder.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTextEdit>

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_picker_machine.h>
#include <qwt_series_data.h>
#include <qwt_text_label.h>
#include <qwt_text.h>
#include <qwt_symbol.h>

QmitkHistogramWidget::QmitkHistogramWidget(QWidget * parent, bool showreport)
  : QDialog(parent)
  , m_Plot(NULL)
  , m_Textedit(NULL)
  , m_Marker(NULL)
  , m_Picker(NULL)
  , m_Zoomer(NULL)
  , m_Histogram(NULL)
{
  QBoxLayout *layout = new QVBoxLayout(this);

  //***histogram***

  QGroupBox *hgroupbox = new QGroupBox("", this);

  hgroupbox->setMinimumSize(900, 400);

  m_Plot = new QwtPlot(hgroupbox);
  m_Plot->setCanvasBackground(QColor(Qt::white));
  m_Plot->setTitle("Histogram");
  QwtText text = m_Plot->titleLabel()->text();
  text.setFont(QFont("Helvetica", 12, QFont::Normal));

  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->enableYMin(true);
  grid->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
  grid->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
  grid->attach(m_Plot);

  layout->addWidget(hgroupbox);

  layout->setSpacing(20);

  if (showreport == true)
  {
    //***report***
    QGroupBox *rgroupbox = new QGroupBox("", this);

    rgroupbox->setMinimumSize(900, 400);

    QLabel *label = new QLabel("Gray  Value  Analysis", rgroupbox);
    label->setAlignment(Qt::AlignHCenter);
    label->setFont(QFont("Helvetica", 14, QFont::Bold));

    m_Textedit = new QTextEdit(rgroupbox);
    m_Textedit->setFont(QFont("Helvetica", 12, QFont::Normal));
    m_Textedit->setReadOnly(true);

    layout->addWidget(rgroupbox);
  }

  m_Picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                               QwtPlotPicker::NoRubberBand, QwtPicker::AlwaysOn,
                               m_Plot->canvas());
  // the m_PickerMachine pointer is managed by the m_Picker instance
  m_Picker->setStateMachine(new QwtPickerClickPointMachine());

  connect(m_Picker, SIGNAL(selected(const QwtDoublePoint &)),
    SLOT(OnSelect(const QwtDoublePoint &)));
}

QmitkHistogramWidget::~QmitkHistogramWidget()
{
}

void QmitkHistogramWidget::SetHistogram(HistogramType::ConstPointer itkHistogram)
{
  HistogramType::SizeType size = itkHistogram->GetSize();
  HistogramType::IndexType index;
  HistogramType::MeasurementVectorType currentMeasurementVector;

  QVector<QwtIntervalSample> intervalSeries(size[0]);

  for (unsigned int i = 0; i < size[0]; ++i)
  {
    index[0] = static_cast<HistogramType::IndexValueType> (i);
    currentMeasurementVector = itkHistogram->GetMeasurementVector(index);
    if (currentMeasurementVector[0] != 0.0)
    {
      intervalSeries[i] = QwtIntervalSample(static_cast<double> (itkHistogram->GetFrequency(index)),
                                            Round(currentMeasurementVector[0]-1), Round(currentMeasurementVector[0]));
    }
  }

  // rebuild the plot
  m_Plot->detachItems();

  m_Histogram = new QmitkHistogram();
  m_Histogram->setColor(Qt::darkCyan);
  m_Histogram->setData(QwtIntervalSeriesData(intervalSeries));
  m_Histogram->attach(m_Plot);

  this->InitializeMarker();
  this->InitializeZoomer();

  m_Plot->replot();
}

void QmitkHistogramWidget::SetHistogram( mitk::Image* mitkImage )
{
  this->SetHistogram(mitkImage->GetStatistics()->GetScalarHistogram());
}

void QmitkHistogramWidget::SetReport(std::string report)
{
  m_Textedit->setText(report.c_str());
}

void QmitkHistogramWidget::InitializeMarker()
{
  m_Marker = new QwtPlotMarker();
  m_Marker->setXValue(0.);
  m_Marker->setLineStyle(QwtPlotMarker::VLine);
  m_Marker->setLabelAlignment(Qt::AlignHCenter | Qt::AlignRight);
  m_Marker->setLinePen(QPen(QColor(200,150,0), 3, Qt::SolidLine));
  m_Marker->setSymbol(new QwtSymbol(QwtSymbol::Diamond,
                                    QColor(Qt::red), QColor(Qt::red), QSize(10,10)));
  m_Marker->attach(m_Plot);
}


void QmitkHistogramWidget::InitializeZoomer()
{
  m_Zoomer = new QwtPlotZoomer(m_Plot->xBottom, m_Plot->yLeft, m_Plot->canvas());
  m_Zoomer->setRubberBandPen(QPen(Qt::red, 2, Qt::DotLine));
  m_Zoomer->setTrackerPen(QPen(Qt::red));
}

void QmitkHistogramWidget::OnSelect( const QPointF& pos )
{
  m_Marker->setXValue( this->Round(pos.x()) );
  QString str = QString( "%1" )
    .arg( (int)(this->Round(pos.x())), 0, 10 );
  QwtText text(str);
  text.setBackgroundBrush(QColor(200,150,0));
  text.setFont(QFont("Helvetica", 14, QFont::Bold));
  m_Marker->setLabel(text);
  m_Plot->replot();
}

double QmitkHistogramWidget::GetMarkerPosition()
{
  return m_Marker->xValue();
}

double QmitkHistogramWidget::Round(double val)
{
  double ival = (double)(int)val;
  if( (val - ival) > 0.5)
    return ival+1;
  else
    return ival;
}

