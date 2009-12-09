#include <qlayout.h>
#include <iostream>

#include "QmitkPlotWidget.h"

QmitkPlotWidget::QmitkPlotWidget(QWidget* parent, const char* title, const char*  /*name*/, Qt::WindowFlags f): QWidget(parent, f) 
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


unsigned int QmitkPlotWidget::InsertCurve(const char* title)
{
  QwtPlotCurve* curve = new QwtPlotCurve(QwtText(title));
  m_PlotCurveVector.push_back(curve);
  curve->attach(m_Plot);
  return static_cast<unsigned int> (m_PlotCurveVector.size() - 1);
}

void QmitkPlotWidget::SetPlotTitle(const char* title)
{
  m_Plot->setTitle(title);
}

void QmitkPlotWidget::SetAxisTitle(int axis, const char* title)
{
  m_Plot->setAxisTitle(axis, title);
}


bool QmitkPlotWidget::SetCurveData( unsigned int curveId, const QmitkPlotWidget::DataVector& xValues, const QmitkPlotWidget::DataVector& yValues )
{
  if ( xValues.size() != yValues.size() )
  {
    std::cerr << "Sizes of data arrays don't match." << std::endl;
    return false;
  }
  double* rawDataX = ConvertToRawArray( xValues );
  double* rawDataY = ConvertToRawArray( yValues );
  m_PlotCurveVector[curveId]->setData( rawDataX, rawDataY, static_cast<int>(xValues.size()) );
  delete[] rawDataX;
  delete[] rawDataY;
  return true;
}


bool QmitkPlotWidget::SetCurveData( unsigned int curveId, const QmitkPlotWidget::XYDataVector& data )
{
  double* rawDataX = ConvertToRawArray( data, 0 );
  double* rawDataY = ConvertToRawArray( data, 1 );
  m_PlotCurveVector[curveId]->setData( rawDataX, rawDataY, static_cast<int>(data.size()) );
  delete[] rawDataX;
  delete[] rawDataY;
  return true;
}

void QmitkPlotWidget::SetCurvePen( unsigned int curveId, const QPen& pen )
{
  m_PlotCurveVector[curveId]->setPen( pen );
}


void QmitkPlotWidget::SetCurveBrush( unsigned int curveId, const QBrush& brush )
{
  m_PlotCurveVector[curveId]->setBrush( brush );
}


void QmitkPlotWidget::SetCurveTitle( unsigned int  /*curveId*/, const char* title )
{
  m_Plot->setTitle( title );
}

void QmitkPlotWidget::SetCurveStyle( unsigned int curveId, const QwtPlotCurve::CurveStyle* style )
{
  m_PlotCurveVector[curveId]->setStyle(*style);
}

void QmitkPlotWidget::SetCurveSymbol( unsigned int curveId, const QwtSymbol* symbol )
{
  m_PlotCurveVector[curveId]->setSymbol(*symbol);
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
