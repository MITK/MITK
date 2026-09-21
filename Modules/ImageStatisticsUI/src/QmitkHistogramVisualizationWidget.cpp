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
#include <qwt_scale_div.h>
#include <qwt_scale_widget.h>
#include <qwt_series_data.h>
#include <qwt_text.h>

#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QColor>
#include <QEvent>
#include <QMouseEvent>
#include <QPen>
#include <QStack>
#include <QVBoxLayout>
#include <QVector>

#include <cmath>
#include <optional>

namespace
{
  const QColor BAR_COLOR(0x4a, 0x90, 0xd9);

  QVector<QwtIntervalSample> ToIntervalSamples(const itk::Statistics::Histogram<double>* histogram)
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

  struct HoveredBar
  {
    const QwtPlotHistogram* item = nullptr;
    QwtIntervalSample sample;
  };

  /** Returns the bar under the cursor. Among the series that have a bin at x this is the
  lowest bar that still reaches up to y, or the tallest bar if the cursor is above all of
  them. The highlight item is skipped because it duplicates the hovered bar. */
  std::optional<HoveredBar> FindHoveredBar(const QwtPlot* plot, const QwtPlotItem* highlightItem, const QPointF& pos)
  {
    std::optional<HoveredBar> covering;
    std::optional<HoveredBar> tallest;

    for (const auto* plotItem : plot->itemList(QwtPlotItem::Rtti_PlotHistogram))
    {
      if (plotItem == highlightItem)
        continue;

      const auto* histogram = static_cast<const QwtPlotHistogram*>(plotItem);
      const int bin = FindBin(histogram->data(), pos.x());

      if (bin < 0)
        continue;

      const HoveredBar bar{ histogram, histogram->data()->sample(bin) };

      if (bar.sample.value >= pos.y() && (!covering || bar.sample.value < covering->sample.value))
        covering = bar;

      if (!tallest || bar.sample.value > tallest->sample.value)
        tallest = bar;
    }

    return covering ? covering : tallest;
  }

  /** Number of shown series, i.e. all histogram items except the highlight item. */
  int CountSeries(const QwtPlot* plot)
  {
    return static_cast<int>(plot->itemList(QwtPlotItem::Rtti_PlotHistogram).size()) - 1;
  }

  /** The value range currently shown by the axes, in the layout of QwtPlotZoomer's rectangles. */
  QRectF VisibleRect(const QwtPlot* plot)
  {
    const QwtScaleDiv& x = plot->axisScaleDiv(QwtPlot::xBottom);
    const QwtScaleDiv& y = plot->axisScaleDiv(QwtPlot::yLeft);
    return QRectF(QPointF(x.lowerBound(), y.lowerBound()), QPointF(x.upperBound(), y.upperBound()));
  }

  bool OverlapsHorizontally(const QRectF& a, const QRectF& b)
  {
    return a.left() < b.right() && b.left() < a.right();
  }

  bool OverlapsVertically(const QRectF& a, const QRectF& b)
  {
    return a.top() < b.bottom() && b.top() < a.bottom();
  }

  /** Shows the hovered bin's series, gray-value range and frequency as a tracker tooltip. */
  class HistogramPicker : public QwtPlotPicker
  {
  public:
    HistogramPicker(QwtPlot* plot, const QwtPlotItem* highlightItem)
      : QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::NoRubberBand, QwtPicker::AlwaysOn, plot->canvas()),
        m_Plot(plot),
        m_HighlightItem(highlightItem)
    {
      this->setStateMachine(new QwtPickerTrackerMachine);
    }

  protected:
    QwtText trackerTextF(const QPointF& pos) const override
    {
      const auto hovered = FindHoveredBar(m_Plot, m_HighlightItem, pos);

      if (!hovered)
        return QwtText();

      QString text;

      if (CountSeries(m_Plot) > 1)
        text = hovered->item->title().text() + '\n';

      text += QString("Gray value: [%1, %2]\nFrequency: %3")
        .arg(hovered->sample.interval.minValue()).arg(hovered->sample.interval.maxValue()).arg(hovered->sample.value);

      return QmitkImageStatisticsPlot::MakeTooltip(text);
    }

  private:
    QwtPlot* m_Plot;
    const QwtPlotItem* m_HighlightItem;
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

  new HistogramPicker(m_Plot, m_HighlightItem);
  m_Plot->canvas()->installEventFilter(this);
  m_Plot->canvas()->setMouseTracking(true);

  // Frequencies are never negative, so no navigation needs to reach below zero.
  m_Zoomer = QmitkImageStatisticsPlot::SetupNavigation(m_Plot, 0.0);

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

void QmitkHistogramVisualizationWidget::SetHistograms(const std::vector<HistogramSeries>& series)
{
  // Panning and wheel zoom change the axes without touching the zoom stack, so
  // the visible range rather than the zoomer's rectangle is what has to survive.
  const QRectF previousView = VisibleRect(m_Plot);
  const bool wasZoomed = m_Zoomer->zoomRectIndex() > 0 || previousView != m_Zoomer->zoomBase();
  const QStack<QRectF> previousZoomStack = m_Zoomer->zoomStack();
  const int previousZoomIndex = static_cast<int>(m_Zoomer->zoomRectIndex());

  this->RemoveSeries();

  // Overlapping series stay visible only with translucent fills. A single
  // series keeps the opaque look.
  const int alpha = series.size() > 1 ? 150 : 255;

  for (const auto& entry : series)
  {
    if (entry.histogram.IsNull())
      continue;

    QColor color = entry.color.isValid() ? entry.color : BAR_COLOR;
    color.setAlpha(alpha);
    auto* item = new QwtPlotHistogram(entry.name);
    item->setStyle(QwtPlotHistogram::Columns);
    // No outline: adjacent columns of a series touch anyway, and an outline
    // would stay visible as a grid where translucent series overlap.
    item->setPen(QPen(Qt::NoPen));
    item->setBrush(color);
    item->setSamples(ToIntervalSamples(entry.histogram));
    item->attach(m_Plot);
    m_Series.push_back({ entry, item });
  }

  if (m_Series.empty())
  {
    // Axes and zoom stack are left alone so that the next series appears in
    // the same view.
    m_Plot->replot();
    this->SetGUIElementsEnabled(false);
    return;
  }

  // A previous zoom leaves fixed axis scales behind. Re-enable autoscaling so
  // the replot fits all series and take that view as the new zoom base.
  m_Plot->setAxisAutoScale(QwtPlot::xBottom);
  m_Plot->setAxisAutoScale(QwtPlot::yLeft);
  m_Zoomer->setZoomBase(true);

  const QRectF base = m_Zoomer->zoomBase();

  // The zoom is about the gray value range, so it survives as long as it still
  // overlaps the new data horizontally. A frequency range that does not (e.g.
  // after switching from a large to a small label) would leave the plot empty
  // and is widened to the new full range instead.
  if (wasZoomed && OverlapsHorizontally(previousView, base))
  {
    QRectF view = previousView;

    if (!OverlapsVertically(view, base))
    {
      view.setTop(base.top());
      view.setBottom(base.bottom());
    }

    // The former view replaces the stack entry it was based on, so panning or
    // the widened y range do not leave a stale rectangle behind.
    QStack<QRectF> zoomStack = m_Zoomer->zoomStack();

    for (int i = 1; i < previousZoomIndex; ++i)
      zoomStack.push(previousZoomStack[i]);

    if (view != zoomStack.top())
      zoomStack.push(view);

    m_Zoomer->setZoomStack(zoomStack, static_cast<int>(zoomStack.size()) - 1);
  }

  this->OnZoomed(m_Zoomer->zoomRect());
  this->SetGUIElementsEnabled(true);
}

void QmitkHistogramVisualizationWidget::Reset()
{
  this->RemoveSeries();

  m_Plot->setAxisAutoScale(QwtPlot::xBottom);
  m_Plot->setAxisAutoScale(QwtPlot::yLeft);
  // Drops the zoom stack too, otherwise the next SetHistograms() would restore
  // a zoom that belongs to data no longer shown.
  m_Zoomer->setZoomBase(true);

  this->SetGUIElementsEnabled(false);
}

void QmitkHistogramVisualizationWidget::RemoveSeries()
{
  for (const auto& entry : m_Series)
  {
    entry.item->detach();
    delete entry.item;
  }

  m_Series.clear();
  this->ClearHighlight();
}

int QmitkHistogramVisualizationWidget::GetBins() {
  return m_Controls->spinBoxNBins->value();
}

void QmitkHistogramVisualizationWidget::ResetDefault()
{
  m_Controls->checkBoxUseDefaultNBins->setChecked(true);
  m_Controls->spinBoxNBins->setEnabled(false);
  m_Controls->spinBoxNBins->setValue(m_DefaultNBins);
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
  const auto hovered = FindHoveredBar(m_Plot, m_HighlightItem, pos);
  const bool hasHighlight = m_HighlightItem->data()->size() > 0;

  if (hovered)
  {
    if (hasHighlight && m_HighlightItem->data()->sample(0) == hovered->sample)
      return;

    QVector<QwtIntervalSample> single;
    single.append(hovered->sample);
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
      const auto position = mouseEvent->position();
      this->OnHover(QPointF(
        m_Plot->invTransform(QwtPlot::xBottom, position.x()),
        m_Plot->invTransform(QwtPlot::yLeft, position.y())));
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
  connect(m_Controls->doubleSpinBoxMinValue, &QDoubleSpinBox::editingFinished, this, &QmitkHistogramVisualizationWidget::OnZoomRangeEdited);
  connect(m_Controls->doubleSpinBoxMaxValue, &QDoubleSpinBox::editingFinished, this, &QmitkHistogramVisualizationWidget::OnZoomRangeEdited);
  connect(m_Controls->buttonResetZoom, &QPushButton::clicked, this, [this]() { m_Zoomer->zoom(0); });
  // The axis rather than the zoomer, so the spin boxes also follow panning and
  // wheel zoom, which bypass the zoom stack.
  connect(m_Plot->axisWidget(QwtPlot::xBottom), &QwtScaleWidget::scaleDivChanged, this, [this]()
  {
    this->OnZoomed(VisibleRect(m_Plot));
  });
}

void QmitkHistogramVisualizationWidget::SetGUIElementsEnabled(bool enabled)
{
  // Children follow the widget's own state; only the bins spin box carries an
  // extra dependency on the default checkbox.
  this->setEnabled(enabled);
  m_Controls->spinBoxNBins->setEnabled(!m_Controls->checkBoxUseDefaultNBins->isChecked());
}

void QmitkHistogramVisualizationWidget::OnClipboardButtonClicked()
{
  if (!m_Series.empty())
  {
    QString clipboard;
    for (const auto& entry : m_Series)
    {
      clipboard.append(entry.series.name);
      clipboard.append("\nMeasurement \t Frequency\n");
      auto iter = entry.series.histogram->Begin();
      auto iterEnd = entry.series.histogram->End();
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

void QmitkHistogramVisualizationWidget::OnZoomRangeEdited()
{
  QRectF rect = m_Zoomer->zoomRect();
  rect.setLeft(m_Controls->doubleSpinBoxMinValue->value());
  rect.setRight(m_Controls->doubleSpinBoxMaxValue->value());

  // Goes through the zoomer so the rectangle lands on its zoom stack and
  // mouse navigation continues from there; zoomed() then syncs the spin boxes.
  m_Zoomer->zoom(rect);
}

void QmitkHistogramVisualizationWidget::OnZoomed(const QRectF& rect)
{
  auto* minBox = m_Controls->doubleSpinBoxMinValue;
  auto* maxBox = m_Controls->doubleSpinBoxMaxValue;
  const QRectF base = m_Zoomer->zoomBase();

  // Widen both boxes to the full range first so neither value is clamped by a
  // limit left over from the previous zoom rectangle.
  minBox->setRange(base.left(), base.right());
  maxBox->setRange(base.left(), base.right());
  minBox->setValue(rect.left());
  maxBox->setValue(rect.right());

  // Keep min strictly below max by at least one displayed unit.
  const double minWidth = std::pow(10.0, -minBox->decimals());
  minBox->setMaximum(maxBox->value() - minWidth);
  maxBox->setMinimum(minBox->value() + minWidth);
}
