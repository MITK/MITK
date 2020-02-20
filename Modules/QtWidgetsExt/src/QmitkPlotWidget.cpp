/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <iostream>
#include <qlayout.h>

#include <qwt_interval_symbol.h>
#include <qwt_point_data.h>

#include "QmitkPlotWidget.h"

QmitkPlotWidget::QmitkPlotWidget(QWidget *parent, const char *title, const char *, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  auto boxLayout = new QVBoxLayout(this);
  m_Plot = new QwtPlot(QwtText(title), this);
  m_Plot->setCanvasBackground(Qt::white);
  boxLayout->addWidget(m_Plot);
}

QmitkPlotWidget::~QmitkPlotWidget()
{
  this->Clear();
  delete m_Plot;
}

QwtPlot *QmitkPlotWidget::GetPlot()
{
  return m_Plot;
}

void QmitkPlotWidget::SetLegend(QwtLegend *legend, QwtPlot::LegendPosition pos, double ratio)
{
  m_Plot->insertLegend(legend, pos, ratio);
}

void QmitkPlotWidget::SetLegendAttribute(unsigned int curveId, const QwtPlotCurve::LegendAttribute &attribute)
{
  std::get<0>(m_PlotCurveVector[curveId])->setLegendAttribute(attribute);
}

unsigned int QmitkPlotWidget::InsertCurve(const char *title, QColor color)
{
  QwtText qwt_title = QwtText(title);
  qwt_title.setColor(color);
  qwt_title.setPaintAttribute(QwtText::PaintUsingTextColor);

  QwtPlotCurve *curve = new QwtPlotCurve(qwt_title);
  QwtPlotIntervalCurve *xErrors = new QwtPlotIntervalCurve();
  QwtPlotIntervalCurve *yErrors = new QwtPlotIntervalCurve();

  auto tuple = std::make_tuple(curve, xErrors, yErrors);
  m_PlotCurveVector.push_back(tuple);

  std::get<0>(m_PlotCurveVector.back())->attach(m_Plot);
  std::get<1>(m_PlotCurveVector.back())->attach(m_Plot);
  std::get<2>(m_PlotCurveVector.back())->attach(m_Plot);

  // error curves should not show up on the legend
  std::get<1>(m_PlotCurveVector.back())->setItemAttribute(QwtPlotItem::Legend, false);
  std::get<2>(m_PlotCurveVector.back())->setItemAttribute(QwtPlotItem::Legend, false);

  return static_cast<unsigned int>(m_PlotCurveVector.size() - 1);
}

void QmitkPlotWidget::SetPlotTitle(const QwtText &qwt_title)
{
  this->m_Plot->setTitle(qwt_title);
}

void QmitkPlotWidget::SetPlotTitle(const char *title)
{
  QwtText qwt_title_text(title);
  this->SetPlotTitle(qwt_title_text);
}

void QmitkPlotWidget::SetAxisTitle(int axis, const char *title)
{
  m_Plot->setAxisTitle(axis, title);
}

bool QmitkPlotWidget::SetCurveData(unsigned int curveId,
                                   const QmitkPlotWidget::DataVector &xValues,
                                   const QmitkPlotWidget::DataVector &yValues)
{
  if (xValues.size() != yValues.size())
  {
    std::cerr << "Sizes of data arrays don't match." << std::endl;
    return false;
  }
  double *rawDataX = ConvertToRawArray(xValues);
  double *rawDataY = ConvertToRawArray(yValues);
  std::get<0>(m_PlotCurveVector[curveId])
    ->setSamples(new QwtPointArrayData(rawDataX, rawDataY, static_cast<int>(xValues.size())));
  delete[] rawDataX;
  delete[] rawDataY;
  return true;
}

bool QmitkPlotWidget::SetCurveData(unsigned int curveId,
                                   const DataVector &xValues,
                                   const DataVector &yValues,
                                   const DataVector &yLowerError,
                                   const DataVector &yUpperError)
{
  bool success = true;
  success = success && this->SetCurveData(curveId, xValues, yValues);
  success = success && this->AddErrorIntervalCurve(curveId, yLowerError, yUpperError, false);

  return success;
}

bool QmitkPlotWidget::SetCurveData(unsigned int curveId,
                                   const DataVector &xValues,
                                   const DataVector &yValues,
                                   const DataVector &xLowerError,
                                   const DataVector &xUpperError,
                                   const DataVector &yLowerError,
                                   const DataVector &yUpperError)
{
  bool success = true;
  success = success && this->SetCurveData(curveId, xValues, yValues);
  success = success && this->AddErrorIntervalCurve(curveId, xLowerError, xUpperError, true);
  success = success && this->AddErrorIntervalCurve(curveId, yLowerError, yUpperError, false);

  return success;
}

bool QmitkPlotWidget::SetCurveData(unsigned int curveId, const XYDataVector &data)
{
  double *rawDataX = ConvertToRawArray(data, 0);
  double *rawDataY = ConvertToRawArray(data, 1);
  std::get<0>(m_PlotCurveVector[curveId])
    ->setData(new QwtPointArrayData(rawDataX, rawDataY, static_cast<int>(data.size())));
  delete[] rawDataX;
  delete[] rawDataY;
  return true;
}

void QmitkPlotWidget::SetCurvePen(unsigned int curveId, const QPen &pen)
{
  std::get<0>(m_PlotCurveVector[curveId])->setPen(pen);
  std::get<0>(m_PlotCurveVector[curveId])->setLegendAttribute(QwtPlotCurve::LegendShowLine);
}

void QmitkPlotWidget::SetCurveBrush(unsigned int curveId, const QBrush &brush)
{
  std::get<0>(m_PlotCurveVector[curveId])->setBrush(brush);
  std::get<0>(m_PlotCurveVector[curveId])->setLegendAttribute(QwtPlotCurve::LegendShowBrush);
}

void QmitkPlotWidget::SetCurveTitle(unsigned int, const char *title)
{
  m_Plot->setTitle(title);
}

void QmitkPlotWidget::SetCurveStyle(unsigned int curveId, const QwtPlotCurve::CurveStyle style)
{
  std::get<0>(m_PlotCurveVector[curveId])->setStyle(style);
}

void QmitkPlotWidget::SetCurveSymbol(unsigned int curveId, QwtSymbol *symbol)
{
  std::get<0>(m_PlotCurveVector[curveId])->setSymbol(symbol);
  std::get<0>(m_PlotCurveVector[curveId])->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
}

void QmitkPlotWidget::SetCurveAntialiasingOn(unsigned int curveId)
{
  std::get<0>(m_PlotCurveVector[curveId])->setRenderHint(QwtPlotItem::RenderAntialiased);
}

void QmitkPlotWidget::SetCurveAntialiasingOff(unsigned int curveId)
{
  std::get<0>(m_PlotCurveVector[curveId])->setRenderHint(QwtPlotItem::RenderAntialiased, false);
}

void QmitkPlotWidget::SetErrorPen(unsigned int curveId, const QPen &pen)
{
  std::get<1>(m_PlotCurveVector[curveId])->setPen(pen);
  QwtIntervalSymbol *errorBar = new QwtIntervalSymbol(QwtIntervalSymbol::Bar);
  errorBar->setPen(pen);
  std::get<1>(m_PlotCurveVector[curveId])->setSymbol(errorBar);
  std::get<2>(m_PlotCurveVector[curveId])->setPen(pen);
  errorBar = new QwtIntervalSymbol(QwtIntervalSymbol::Bar);
  errorBar->setPen(pen);
  std::get<2>(m_PlotCurveVector[curveId])->setSymbol(errorBar);
}

void QmitkPlotWidget::SetErrorStyleSymbols(unsigned int curveId, bool drawSmybols)
{
  if (drawSmybols)
  {
    std::get<1>(m_PlotCurveVector[curveId])->setStyle(QwtPlotIntervalCurve::NoCurve);
    QwtIntervalSymbol *errorBar = new QwtIntervalSymbol(QwtIntervalSymbol::Bar);
    errorBar->setPen(std::get<1>(m_PlotCurveVector[curveId])->pen());
    std::get<1>(m_PlotCurveVector[curveId])->setSymbol(errorBar);
    std::get<2>(m_PlotCurveVector[curveId])->setStyle(QwtPlotIntervalCurve::NoCurve);
    errorBar = new QwtIntervalSymbol(QwtIntervalSymbol::Bar);
    errorBar->setPen(std::get<2>(m_PlotCurveVector[curveId])->pen());
    std::get<2>(m_PlotCurveVector[curveId])->setSymbol(errorBar);
  }
  else
  {
    std::get<1>(m_PlotCurveVector[curveId])->setStyle(QwtPlotIntervalCurve::Tube);
    std::get<1>(m_PlotCurveVector[curveId])->setSymbol(nullptr);
    std::get<2>(m_PlotCurveVector[curveId])->setStyle(QwtPlotIntervalCurve::Tube);
    std::get<2>(m_PlotCurveVector[curveId])->setSymbol(nullptr);
  }
}

void QmitkPlotWidget::Replot()
{
  m_Plot->replot();
}

void QmitkPlotWidget::Clear()
{
  m_Plot->detachItems();
  m_PlotCurveVector.clear();
  m_PlotCurveVector.resize(0);
}

double *QmitkPlotWidget::ConvertToRawArray(const QmitkPlotWidget::DataVector &values)
{
  auto raw = new double[values.size()];
  for (unsigned int i = 0; i < values.size(); ++i)
    raw[i] = values[i];
  return raw;
}

double *QmitkPlotWidget::ConvertToRawArray(const QmitkPlotWidget::XYDataVector &values, unsigned int component)
{
  auto raw = new double[values.size()];
  for (unsigned int i = 0; i < values.size(); ++i)
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
        std::cout << "Component must be either 0 or 1." << std::endl;
    }
  }
  return raw;
}

bool QmitkPlotWidget::AddErrorIntervalCurve(unsigned int curveId,
                                            const DataVector &lessError,
                                            const DataVector &moreError,
                                            bool isXError)
{
  const QwtSeriesData<QPointF> *curveSeriesData = std::get<0>(this->m_PlotCurveVector[curveId])->data();

  size_t size = curveSeriesData->size();

  if (size != lessError.size() || size != moreError.size())
  {
    std::cerr << "Sizes of data arrays don't match." << std::endl;
    return false;
  }

  QVector<QwtIntervalSample> samples;
  QwtIntervalSample *sample;
  QwtPlotIntervalCurve *curve;

  if (isXError)
  {
    curve = std::get<1>(m_PlotCurveVector[curveId]);
  }
  else
  {
    curve = std::get<2>(m_PlotCurveVector[curveId]);
  }

  for (unsigned int index = 0; index < size; ++index)
  {
    qreal xValue = curveSeriesData->sample(index).x();
    qreal yValue = curveSeriesData->sample(index).y();
    if (isXError)
    {
      sample = new QwtIntervalSample(xValue, xValue - lessError[index], xValue + moreError[index]);
    }
    else
    {
      sample = new QwtIntervalSample(xValue, yValue - lessError[index], yValue + moreError[index]);
    }
    samples.push_back(*sample);
  }

  curve->setSamples(samples);
  curve->setStyle(QwtPlotIntervalCurve::NoCurve);

  QwtIntervalSymbol *errorBar = new QwtIntervalSymbol(QwtIntervalSymbol::Bar);
  errorBar->setPen(QPen(Qt::black));
  curve->setSymbol(errorBar);

  if (isXError)
  {
    curve->setOrientation(Qt::Horizontal);
  }
  else
  {
    curve->setOrientation(Qt::Vertical);
  }

  return true;
}
