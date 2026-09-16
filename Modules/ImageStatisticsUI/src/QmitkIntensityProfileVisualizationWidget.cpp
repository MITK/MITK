/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkIntensityProfileVisualizationWidget.h>
#include <ui_QmitkIntensityProfileVisualizationWidget.h>

#include <QmitkPlotWidget.h>

#include "QmitkImageStatisticsPlotHelper.h"

#include <qwt_picker_machine.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_item.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_series_data.h>
#include <qwt_text.h>

#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QPen>
#include <QVBoxLayout>

namespace
{
  const QColor LINE_COLOR(0x4a, 0x90, 0xd9);

  /** Shows the intensity at the hovered sample as a tracker tooltip. */
  class ProfilePicker : public QwtPlotPicker
  {
  public:
    explicit ProfilePicker(QwtPlot* plot)
      : QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::NoRubberBand, QwtPicker::AlwaysOn, plot->canvas()),
        m_Plot(plot)
    {
      this->setStateMachine(new QwtPickerTrackerMachine);
    }

  protected:
    QwtText trackerTextF(const QPointF& pos) const override
    {
      const QwtPlotItemList items = m_Plot->itemList(QwtPlotItem::Rtti_PlotCurve);
      if (items.isEmpty())
        return QwtText();

      const auto* curve = static_cast<const QwtPlotCurve*>(items.first());
      const QwtSeriesData<QPointF>* data = curve->data();
      if (data->size() == 0)
        return QwtText();

      const int index = qBound(0, qRound(pos.x()), static_cast<int>(data->size()) - 1);
      const QPointF point = data->sample(index);
      return QmitkImageStatisticsPlot::MakeTooltip(QString("Distance: %1\nIntensity: %2").arg(point.x()).arg(point.y()));
    }

  private:
    QwtPlot* m_Plot;
  };
}

QmitkIntensityProfileVisualizationWidget::QmitkIntensityProfileVisualizationWidget(QWidget* parent)
  : QWidget(parent),
    m_Controls(std::make_unique<Ui::QmitkIntensityProfileControls>()),
    m_PlotWidget(nullptr),
    m_Zoomer(nullptr)
{
  m_Controls->setupUi(this);

  m_PlotWidget = new QmitkPlotWidget(this);
  m_PlotWidget->SetAxisTitle(QwtPlot::xBottom, "Distance");
  m_PlotWidget->SetAxisTitle(QwtPlot::yLeft, "Intensity");

  new ProfilePicker(m_PlotWidget->GetPlot());

  m_Zoomer = QmitkImageStatisticsPlot::SetupNavigation(m_PlotWidget->GetPlot());

  auto* layout = new QVBoxLayout(m_Controls->plotContainer);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_PlotWidget);

  this->ApplyTheme();
  this->SetGUIElementsEnabled(false);
  this->CreateConnections();
}

QmitkIntensityProfileVisualizationWidget::~QmitkIntensityProfileVisualizationWidget()
{
}

void QmitkIntensityProfileVisualizationWidget::SetIntensityProfile(mitk::IntensityProfile::ConstPointer intensityProfile, const std::string& dataLabel)
{
  if (intensityProfile == nullptr)
    return;

  m_IntensityProfileList = mitk::CreateVectorFromIntensityProfile(intensityProfile);
  if (m_IntensityProfileList.empty())
    return;

  std::vector<double> distances(m_IntensityProfileList.size());
  for (std::size_t i = 0; i < distances.size(); ++i)
    distances[i] = static_cast<double>(i);

  const unsigned int curveId = m_PlotWidget->InsertCurve(dataLabel.c_str());
  m_PlotWidget->SetCurveData(curveId, distances, m_IntensityProfileList);
  m_PlotWidget->SetCurveStyle(curveId, QwtPlotCurve::Lines);
  m_PlotWidget->SetCurvePen(curveId, QPen(LINE_COLOR));
  m_PlotWidget->Replot();

  QRectF bounds;
  bool first = true;
  const QwtPlotItemList curves = m_PlotWidget->GetPlot()->itemList(QwtPlotItem::Rtti_PlotCurve);
  for (auto* item : curves)
  {
    const QRectF itemBounds = static_cast<QwtPlotCurve*>(item)->boundingRect();
    bounds = first ? itemBounds : bounds.united(itemBounds);
    first = false;
  }
  if (!curves.isEmpty() && bounds.isValid())
    m_Zoomer->setZoomBase(bounds);

  this->SetGUIElementsEnabled(true);
}

void QmitkIntensityProfileVisualizationWidget::Reset()
{
  m_PlotWidget->Clear();
  m_PlotWidget->Replot();
  m_IntensityProfileList.clear();
  this->SetGUIElementsEnabled(false);
}

void QmitkIntensityProfileVisualizationWidget::SetTheme(QmitkPlotStyle style)
{
  m_Style = style;
  this->ApplyTheme();
}

void QmitkIntensityProfileVisualizationWidget::ApplyTheme()
{
  QmitkImageStatisticsPlot::ApplyTheme(m_PlotWidget->GetPlot(), m_Style);
}

void QmitkIntensityProfileVisualizationWidget::CreateConnections()
{
  connect(m_Controls->buttonCopyToClipboard, &QPushButton::clicked, this, &QmitkIntensityProfileVisualizationWidget::OnClipboardButtonClicked);
}

void QmitkIntensityProfileVisualizationWidget::SetGUIElementsEnabled(bool enabled)
{
  this->setEnabled(enabled);
  m_PlotWidget->setEnabled(enabled);
  m_Controls->buttonCopyToClipboard->setEnabled(enabled);
}

void QmitkIntensityProfileVisualizationWidget::OnClipboardButtonClicked()
{
  if (m_IntensityProfileList.empty())
    return;

  QApplication::clipboard()->clear();

  QString clipboard("Pixel \t Intensity\n");
  for (unsigned int i = 0; i < m_IntensityProfileList.size(); i++)
  {
    clipboard = clipboard.append("%L1 \t %L2\n")
      .arg(QString::number(i))
      .arg(m_IntensityProfileList.at(i),0,'f');
  }
  QApplication::clipboard()->setText(clipboard, QClipboard::Clipboard);
}
