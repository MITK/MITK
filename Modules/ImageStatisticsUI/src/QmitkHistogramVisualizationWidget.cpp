/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkHistogramVisualizationWidget.h>
#include <ui_QmitkHistogramVisualizationWidget.h>

#include "QmitkImageStatisticsPlotHelper.h"

#include <qwt_interval.h>
#include <qwt_picker_machine.h>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_item.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_samples.h>
#include <qwt_series_data.h>
#include <qwt_text.h>

#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QColor>
#include <QEvent>
#include <QMouseEvent>
#include <QPen>
#include <QVBoxLayout>
#include <QVector>

#include <limits>

namespace
{
  const QColor BAR_COLOR(0x4a, 0x90, 0xd9);

  QVector<QwtIntervalSample> ToIntervalSamples(itk::Statistics::Histogram<double>::ConstPointer histogram)
  {
    QVector<QwtIntervalSample> samples;
    samples.reserve(static_cast<int>(histogram->Size()));

    for (auto it = histogram->Begin(); it != histogram->End(); ++it)
    {
      const auto bin = it.GetInstanceIdentifier();
      samples.append(QwtIntervalSample(
        it.GetFrequency(), QwtInterval(histogram->GetBinMin(0, bin), histogram->GetBinMax(0, bin))));
    }

    return samples;
  }

  // Bins are contiguous and ascending, so a binary search finds the hovered
  // bin without scanning every sample (a histogram can hold thousands of bins).
  int FindBin(const QwtSeriesData<QwtIntervalSample>* data, double x)
  {
    int lo = 0;
    int hi = static_cast<int>(data->size()) - 1;

    while (lo <= hi)
    {
      const int mid = (lo + hi) / 2;
      const QwtInterval interval = data->sample(mid).interval;

      if (x < interval.minValue())
        hi = mid - 1;
      else if (x > interval.maxValue())
        lo = mid + 1;
      else
        return mid;
    }

    return -1;
  }

  /** Shows the hovered bin's gray-value range and frequency as a tracker tooltip. */
  class HistogramPicker : public QwtPlotPicker
  {
  public:
    explicit HistogramPicker(QwtPlot* plot)
      : QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::NoRubberBand, QwtPicker::AlwaysOn, plot->canvas()),
        m_Plot(plot)
    {
      this->setStateMachine(new QwtPickerTrackerMachine);
    }

  protected:
    QwtText trackerTextF(const QPointF& pos) const override
    {
      const QwtPlotItemList items = m_Plot->itemList(QwtPlotItem::Rtti_PlotHistogram);
      for (auto* plotItem : items)
      {
        const auto* histogram = static_cast<const QwtPlotHistogram*>(plotItem);
        const QwtSeriesData<QwtIntervalSample>* data = histogram->data();
        const int bin = FindBin(data, pos.x());
        if (bin >= 0)
        {
          const QwtIntervalSample sample = data->sample(bin);
          return QmitkImageStatisticsPlot::MakeTooltip(QString("Gray value: [%1, %2]\nFrequency: %3")
            .arg(sample.interval.minValue()).arg(sample.interval.maxValue()).arg(sample.value));
        }
      }
      return QwtText();
    }

  private:
    QwtPlot* m_Plot;
  };
}

QmitkHistogramVisualizationWidget::QmitkHistogramVisualizationWidget(QWidget* parent)
  : QWidget(parent),
    m_Controls(std::make_unique<Ui::QmitkHistogramVisualizationControls>()),
    m_Plot(nullptr),
    m_HighlightItem(nullptr),
    m_Zoomer(nullptr)
{
  m_Controls->setupUi(this);
  m_Controls->spinBoxNBins->setValue(m_DefaultNBins);
  m_Controls->spinBoxNBins->setMinimum(m_MinNBins);
  m_Controls->spinBoxNBins->setMaximum(m_MaxNBins);

  m_Plot = new QwtPlot(this);
  m_Plot->setAxisTitle(QwtPlot::xBottom, "Gray value");
  m_Plot->setAxisTitle(QwtPlot::yLeft, "Frequency");

  auto* grid = new QwtPlotGrid;
  grid->setPen(QColor(0x88, 0x88, 0x88), 0.0, Qt::DotLine);
  grid->attach(m_Plot);

  m_HighlightItem = new QwtPlotHistogram;
  m_HighlightItem->setStyle(QwtPlotHistogram::Columns);
  m_HighlightItem->setBrush(QmitkImageStatisticsPlot::HIGHLIGHT_COLOR);
  m_HighlightItem->setPen(QPen(QmitkImageStatisticsPlot::HIGHLIGHT_COLOR));
  m_HighlightItem->setZ(1000);
  m_HighlightItem->attach(m_Plot);

  new HistogramPicker(m_Plot);
  m_Plot->canvas()->installEventFilter(this);
  m_Plot->canvas()->setMouseTracking(true);

  m_Zoomer = QmitkImageStatisticsPlot::SetupNavigation(m_Plot);

  auto* layout = new QVBoxLayout(m_Controls->plotContainer);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_Plot);

  this->ApplyTheme();
  this->SetGUIElementsEnabled(false);
  this->CreateConnections();
}

QmitkHistogramVisualizationWidget::~QmitkHistogramVisualizationWidget()
{
}

void QmitkHistogramVisualizationWidget::SetHistogram(itk::Statistics::Histogram<double>::ConstPointer histogram, const std::string& dataLabel)
{
  if (histogram.IsNull())
    return;

  this->ClearHighlight();

  const bool histogramWasEmpty = m_Histograms.empty();
  m_Histograms[dataLabel] = histogram;

  auto it = m_HistogramItems.find(dataLabel);
  if (it == m_HistogramItems.end())
  {
    auto* item = new QwtPlotHistogram(QString::fromStdString(dataLabel));
    item->setStyle(QwtPlotHistogram::Columns);
    item->setBrush(BAR_COLOR);
    item->setPen(QPen(BAR_COLOR));
    item->attach(m_Plot);
    it = m_HistogramItems.emplace(dataLabel, item).first;
  }

  it->second->setSamples(ToIntervalSamples(histogram));
  m_Plot->replot();

  QRectF bounds;
  bool first = true;
  for (const auto& entry : m_HistogramItems)
  {
    const QRectF itemBounds = entry.second->boundingRect();
    bounds = first ? itemBounds : bounds.united(itemBounds);
    first = false;
  }
  if (!m_HistogramItems.empty() && bounds.isValid())
    m_Zoomer->setZoomBase(bounds);

  if (m_Histograms.empty() != histogramWasEmpty)
    this->SetGUIElementsEnabled(!m_Histograms.empty());
}

void QmitkHistogramVisualizationWidget::Reset()
{
  for (const auto& item : m_HistogramItems)
  {
    item.second->detach();
    delete item.second;
  }
  m_HistogramItems.clear();
  m_Histograms.clear();
  this->ClearHighlight();

  m_Plot->setAxisAutoScale(QwtPlot::xBottom);
  m_Plot->replot();

  SetGUIElementsEnabled(false);
}

int QmitkHistogramVisualizationWidget::GetBins() {
  return m_Controls->spinBoxNBins->value();
}

void QmitkHistogramVisualizationWidget::ResetDefault()
{
  m_Controls->checkBoxUseDefaultNBins->setChecked(true);
  m_Controls->spinBoxNBins->setEnabled(false);
  m_Controls->spinBoxNBins->setValue(100);
}

void QmitkHistogramVisualizationWidget::SetTheme(QmitkPlotStyle style)
{
  m_Style = style;
  this->ApplyTheme();
}

void QmitkHistogramVisualizationWidget::ApplyTheme()
{
  QmitkImageStatisticsPlot::ApplyTheme(m_Plot, m_Style);
}

void QmitkHistogramVisualizationWidget::OnHover(const QPointF& pos)
{
  bool found = false;
  QwtIntervalSample hovered;

  for (const auto& entry : m_HistogramItems)
  {
    const QwtSeriesData<QwtIntervalSample>* data = entry.second->data();
    const int bin = FindBin(data, pos.x());
    if (bin >= 0)
    {
      hovered = data->sample(bin);
      found = true;
      break;
    }
  }

  const bool hasHighlight = m_HighlightItem->data()->size() > 0;

  if (found)
  {
    if (hasHighlight && m_HighlightItem->data()->sample(0).interval.minValue() == hovered.interval.minValue())
      return;

    QVector<QwtIntervalSample> single;
    single.append(hovered);
    m_HighlightItem->setSamples(single);
    m_Plot->replot();
  }
  else if (hasHighlight)
  {
    m_HighlightItem->setSamples(QVector<QwtIntervalSample>());
    m_Plot->replot();
  }
}

void QmitkHistogramVisualizationWidget::ClearHighlight()
{
  if (m_HighlightItem->data()->size() > 0)
    m_HighlightItem->setSamples(QVector<QwtIntervalSample>());
}

bool QmitkHistogramVisualizationWidget::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == m_Plot->canvas())
  {
    if (event->type() == QEvent::MouseMove)
    {
      const auto* mouseEvent = static_cast<QMouseEvent*>(event);
      this->OnHover(QPointF(m_Plot->invTransform(QwtPlot::xBottom, mouseEvent->position().x()), 0.0));
    }
    else if (event->type() == QEvent::Leave && m_HighlightItem->data()->size() > 0)
    {
      m_HighlightItem->setSamples(QVector<QwtIntervalSample>());
      m_Plot->replot();
    }
  }

  return QWidget::eventFilter(watched, event);
}

void QmitkHistogramVisualizationWidget::CreateConnections()
{
  connect(m_Controls->buttonCopyHistogramToClipboard, &QPushButton::clicked, this, &QmitkHistogramVisualizationWidget::OnClipboardButtonClicked);
  connect(m_Controls->checkBoxUseDefaultNBins, &QCheckBox::clicked, this, &QmitkHistogramVisualizationWidget::OnDefaultNBinsCheckBoxChanged);
  connect(m_Controls->spinBoxNBins, &QSpinBox::editingFinished, this, &QmitkHistogramVisualizationWidget::OnNBinsSpinBoxValueChanged);
  connect(m_Controls->checkBoxViewMinMax, &QCheckBox::clicked, this, &QmitkHistogramVisualizationWidget::OnViewMinMaxCheckBoxChanged);
  connect(m_Controls->doubleSpinBoxMaxValue, &QSpinBox::editingFinished, this, &QmitkHistogramVisualizationWidget::OnMaxValueSpinBoxValueChanged);
  connect(m_Controls->doubleSpinBoxMinValue, &QSpinBox::editingFinished, this, &QmitkHistogramVisualizationWidget::OnMinValueSpinBoxValueChanged);
}

void QmitkHistogramVisualizationWidget::SetGUIElementsEnabled(bool enabled)
{
  this->setEnabled(enabled);
  m_Controls->tabWidgetPlot->setEnabled(enabled);
  m_Controls->checkBoxUseDefaultNBins->setEnabled(enabled);
  m_Controls->spinBoxNBins->setEnabled(!m_Controls->checkBoxUseDefaultNBins->isChecked());
  m_Controls->buttonCopyHistogramToClipboard->setEnabled(enabled);
  m_Controls->checkBoxViewMinMax->setEnabled(enabled);
  m_Controls->doubleSpinBoxMaxValue->setEnabled(m_Controls->checkBoxViewMinMax->isChecked());
  m_Controls->doubleSpinBoxMinValue->setEnabled(m_Controls->checkBoxViewMinMax->isChecked());
}

void QmitkHistogramVisualizationWidget::OnClipboardButtonClicked()
{
  if (!m_Histograms.empty())
  {
    QString clipboard;
    for (const auto& histogram : m_Histograms)
    {
      clipboard.append(QString::fromStdString(histogram.first));
      clipboard.append("Measurement \t Frequency\n");
      auto iter = histogram.second->Begin();
      auto iterEnd = histogram.second->End();
      for (; iter != iterEnd; ++iter)
      {
        clipboard = clipboard.append("%L1 \t %L2\n")
            .arg(iter.GetMeasurementVector()[0], 0, 'f', 2)
            .arg(iter.GetFrequency());
      }
      clipboard.append("\n\n");
    }

    QApplication::clipboard()->clear();
    QApplication::clipboard()->setText(clipboard, QClipboard::Clipboard);
  }
}

void QmitkHistogramVisualizationWidget::OnDefaultNBinsCheckBoxChanged()
{
  if (m_Controls->checkBoxUseDefaultNBins->isChecked())
  {
    m_Controls->spinBoxNBins->setEnabled(false);
    if (m_Controls->spinBoxNBins->value() != static_cast<int>(m_DefaultNBins) ) {
      m_Controls->spinBoxNBins->setValue(m_DefaultNBins);
      OnNBinsSpinBoxValueChanged();
    }
  }
  else
  {
    m_Controls->spinBoxNBins->setEnabled(true);
  }
}

void QmitkHistogramVisualizationWidget::OnNBinsSpinBoxValueChanged()
{
  emit RequestHistogramUpdate(m_Controls->spinBoxNBins->value());
}

void QmitkHistogramVisualizationWidget::OnViewMinMaxCheckBoxChanged()
{
  double min = std::numeric_limits<double>::max();
  double max = std::numeric_limits<double>::lowest();
  for (const auto& histogram : m_Histograms)
  {
    auto aMin = histogram.second->GetBinMin(0, 0);
    if (min > aMin) min = aMin;

    auto maxVector = histogram.second->GetDimensionMaxs(0);
    if (m_Controls->checkBoxUseDefaultNBins->isChecked())
    {
      max = std::max(max, maxVector[m_DefaultNBins - 1]);
    }
    else
    {
      max = std::max(max, maxVector[m_Controls->spinBoxNBins->value() - 1]);
    }
  }

  if (!m_Controls->checkBoxViewMinMax->isChecked())
  {
    m_Controls->doubleSpinBoxMaxValue->setEnabled(false);
    m_Controls->doubleSpinBoxMinValue->setEnabled(false);
    m_Plot->setAxisAutoScale(QwtPlot::xBottom);
    m_Plot->replot();
  }
  else
  {
    m_Controls->doubleSpinBoxMinValue->setMinimum(min);
    m_Controls->doubleSpinBoxMinValue->setValue(min);
    m_Controls->doubleSpinBoxMaxValue->setMaximum(max);
    m_Controls->doubleSpinBoxMaxValue->setValue(max);
    m_Controls->doubleSpinBoxMaxValue->setEnabled(true);
    m_Controls->doubleSpinBoxMinValue->setEnabled(true);
  }
}

void QmitkHistogramVisualizationWidget::OnMinValueSpinBoxValueChanged()
{
  m_Controls->doubleSpinBoxMaxValue->setMinimum(m_Controls->doubleSpinBoxMinValue->value()+1);
  m_Plot->setAxisScale(QwtPlot::xBottom, m_Controls->doubleSpinBoxMinValue->value(), m_Controls->doubleSpinBoxMaxValue->value());
  m_Plot->replot();
}

void QmitkHistogramVisualizationWidget::OnMaxValueSpinBoxValueChanged()
{
  m_Controls->doubleSpinBoxMinValue->setMaximum(m_Controls->doubleSpinBoxMaxValue->value()-1);
  m_Plot->setAxisScale(QwtPlot::xBottom, m_Controls->doubleSpinBoxMinValue->value(), m_Controls->doubleSpinBoxMaxValue->value());
  m_Plot->replot();
}
