/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkHistogramVisualizationWidget.h>

#include "QmitkTestQApplication.h"

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <qwt_plot.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_div.h>
#include <qwt_text.h>

#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QDoubleSpinBox>
#include <QPen>
#include <QPushButton>
#include <QStack>

class QmitkHistogramVisualizationWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkHistogramVisualizationWidgetTestSuite);
  MITK_TEST(SetHistograms_OneItemPerSeriesInItsColor);
  MITK_TEST(SetHistograms_SingleSeriesIsOpaque);
  MITK_TEST(SetHistograms_ReplacesPreviousSeries);
  MITK_TEST(SetHistograms_KeepsZoom);
  MITK_TEST(SetHistograms_NonIntersectingZoomFallsBackToFullRange);
  MITK_TEST(SetHistograms_KeepsGrayValueRangeWhenFrequenciesDiffer);
  MITK_TEST(SetHistograms_KeepsPannedView);
  MITK_TEST(SetHistograms_DropsZoomStepsOutsideNewData);
  MITK_TEST(SetHistograms_EmptyDisablesControls);
  MITK_TEST(SetHistograms_EmptyKeepsZoomForNextSeries);
  MITK_TEST(Reset_RemovesSeriesAndZoom);
  MITK_TEST(Zoom_NeverBelowZero);
  MITK_TEST(Zoom_ResetAfterPanReturnsToFullRange);
  MITK_TEST(ZoomRangeFields_FollowViewBeyondData);
  MITK_TEST(ZoomRangeEdited_KeepsPannedFrequencyRange);
  MITK_TEST(Clipboard_OneBlockPerSeries);
  CPPUNIT_TEST_SUITE_END();

  using HistogramType = QmitkHistogramVisualizationWidget::HistogramType;
  using HistogramSeries = QmitkHistogramVisualizationWidget::HistogramSeries;

public:

  void setUp() override
  {
    EnsureQApplication();
  }

  /** Frequencies ramp up to maxFrequency in the last bin. A flat histogram would have a
  data rectangle of zero height, for which Qwt does not include the baseline in the
  autoscaled y range, unlike for any real histogram. */
  static HistogramType::ConstPointer CreateHistogram(double lower, double upper, unsigned int bins, double maxFrequency)
  {
    auto histogram = HistogramType::New();
    histogram->SetMeasurementVectorSize(1);

    HistogramType::SizeType size(1);
    size.Fill(bins);
    HistogramType::MeasurementVectorType lowerBound(1);
    lowerBound.Fill(lower);
    HistogramType::MeasurementVectorType upperBound(1);
    upperBound.Fill(upper);
    histogram->Initialize(size, lowerBound, upperBound);

    for (unsigned int bin = 0; bin < bins; ++bin)
      histogram->SetFrequency(bin, static_cast<HistogramType::AbsoluteFrequencyType>(maxFrequency * (bin + 1) / bins));

    return histogram.GetPointer();
  }

  static HistogramSeries Series(const char* name, const QColor& color, double lower, double upper, double maxFrequency = 5.0)
  {
    return { QString(name), color, CreateHistogram(lower, upper, 10, maxFrequency) };
  }

  static void AssertRect(const QRectF& expected, const QRectF& actual)
  {
    CPPUNIT_ASSERT_EQUAL(expected.left(), actual.left());
    CPPUNIT_ASSERT_EQUAL(expected.right(), actual.right());
    CPPUNIT_ASSERT_EQUAL(expected.top(), actual.top());
    CPPUNIT_ASSERT_EQUAL(expected.bottom(), actual.bottom());
  }

  /** All histogram items of the plot except the title-less highlight item, in drawing order. */
  static QList<QwtPlotHistogram*> SeriesItems(QmitkHistogramVisualizationWidget& widget)
  {
    auto* plot = widget.findChild<QwtPlot*>();
    CPPUNIT_ASSERT_MESSAGE("The widget must own a plot.", nullptr != plot);

    QList<QwtPlotHistogram*> items;

    for (auto* item : plot->itemList(QwtPlotItem::Rtti_PlotHistogram))
    {
      auto* histogram = static_cast<QwtPlotHistogram*>(item);

      if (!histogram->title().text().isEmpty())
        items.append(histogram);
    }

    return items;
  }

  static QwtPlotZoomer* Zoomer(QmitkHistogramVisualizationWidget& widget)
  {
    auto* zoomer = widget.findChild<QwtPlotZoomer*>();
    CPPUNIT_ASSERT_MESSAGE("The widget must own a zoomer.", nullptr != zoomer);

    return zoomer;
  }

  /** Moves an axis the way the panner and the magnifier do: behind the zoom stack's back. */
  static void PanTo(QmitkHistogramVisualizationWidget& widget, double lower, double upper, QwtPlot::Axis axis = QwtPlot::xBottom)
  {
    auto* plot = widget.findChild<QwtPlot*>();
    CPPUNIT_ASSERT_MESSAGE("The widget must own a plot.", nullptr != plot);

    plot->setAxisScale(axis, lower, upper);
    plot->replot();
  }

  static double SpinBoxValue(QmitkHistogramVisualizationWidget& widget, const char* name)
  {
    auto* spinBox = widget.findChild<QDoubleSpinBox*>(name);
    CPPUNIT_ASSERT_MESSAGE("Spin box not found.", nullptr != spinBox);

    return spinBox->value();
  }

  void SetHistograms_OneItemPerSeriesInItsColor()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("Red", QColor(Qt::red), 0.0, 10.0), Series("Default", QColor(), 0.0, 10.0) });

    const auto items = SeriesItems(widget);
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(items.size()));
    CPPUNIT_ASSERT_EQUAL(std::string("Red"), items[0]->title().text().toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("Default"), items[1]->title().text().toStdString());
    CPPUNIT_ASSERT_MESSAGE("Columns must not have an outline.", Qt::NoPen == items[0]->pen().style());
    CPPUNIT_ASSERT_EQUAL(QColor(Qt::red).rgb(), items[0]->brush().color().rgb());
    CPPUNIT_ASSERT_MESSAGE("Overlapping series must be translucent.", items[0]->brush().color().alpha() < 255);
    CPPUNIT_ASSERT_MESSAGE("A series without a color must fall back to the default color.",
      items[1]->brush().color().isValid() && items[1]->brush().color().rgb() != QColor(Qt::red).rgb());
    CPPUNIT_ASSERT(widget.isEnabled());
  }

  void SetHistograms_SingleSeriesIsOpaque()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("Only", QColor(Qt::green), 0.0, 10.0) });

    const auto items = SeriesItems(widget);
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(items.size()));
    CPPUNIT_ASSERT_EQUAL(255, items[0]->brush().color().alpha());
  }

  void SetHistograms_ReplacesPreviousSeries()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(Qt::red), 0.0, 10.0), Series("B", QColor(Qt::green), 0.0, 10.0) });
    widget.SetHistograms({ Series("C", QColor(Qt::blue), 0.0, 10.0) });

    const auto items = SeriesItems(widget);
    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(items.size()));
    CPPUNIT_ASSERT_EQUAL(std::string("C"), items[0]->title().text().toStdString());
  }

  void SetHistograms_KeepsZoom()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });

    auto* zoomer = Zoomer(widget);
    const QRectF zoomRect(20.0, 0.0, 30.0, 3.0);
    zoomer->zoom(zoomRect);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: not zoomed in.", 1u, zoomer->zoomRectIndex());

    widget.SetHistograms({ Series("B", QColor(), 0.0, 200.0, 8.0) });

    CPPUNIT_ASSERT_EQUAL(1u, zoomer->zoomRectIndex());
    AssertRect(zoomRect, zoomer->zoomRect());
    CPPUNIT_ASSERT_MESSAGE("The zoom base must cover the new data.", zoomer->zoomBase().right() >= 200.0);
  }

  void SetHistograms_NonIntersectingZoomFallsBackToFullRange()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });

    auto* zoomer = Zoomer(widget);
    zoomer->zoom(QRectF(60.0, 0.0, 20.0, 3.0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: not zoomed in.", 1u, zoomer->zoomRectIndex());

    widget.SetHistograms({ Series("B", QColor(), 1000.0, 1100.0) });

    CPPUNIT_ASSERT_EQUAL(0u, zoomer->zoomRectIndex());
    CPPUNIT_ASSERT(zoomer->zoomRect().left() <= 1000.0);
    CPPUNIT_ASSERT(zoomer->zoomRect().right() >= 1100.0);
  }

  void SetHistograms_KeepsGrayValueRangeWhenFrequenciesDiffer()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("Large", QColor(), 0.0, 100.0, 5000.0) });

    auto* zoomer = Zoomer(widget);
    zoomer->zoom(QRectF(20.0, 2000.0, 30.0, 2000.0));

    // The small label's frequencies do not reach the zoomed y range, but its
    // gray values do overlap the zoomed x range.
    widget.SetHistograms({ Series("Small", QColor(), 0.0, 100.0, 5.0) });

    CPPUNIT_ASSERT_EQUAL(20.0, zoomer->zoomRect().left());
    CPPUNIT_ASSERT_EQUAL(50.0, zoomer->zoomRect().right());
    CPPUNIT_ASSERT_EQUAL(zoomer->zoomBase().top(), zoomer->zoomRect().top());
    CPPUNIT_ASSERT_EQUAL(zoomer->zoomBase().bottom(), zoomer->zoomRect().bottom());
  }

  void SetHistograms_KeepsPannedView()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });

    PanTo(widget, 30.0, 60.0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The spin boxes must follow the axis.", 30.0, SpinBoxValue(widget, "doubleSpinBoxMinValue"));
    CPPUNIT_ASSERT_EQUAL(60.0, SpinBoxValue(widget, "doubleSpinBoxMaxValue"));

    widget.SetHistograms({ Series("B", QColor(), 0.0, 100.0, 8.0) });

    auto* zoomer = Zoomer(widget);
    CPPUNIT_ASSERT_EQUAL(1u, zoomer->zoomRectIndex());
    CPPUNIT_ASSERT_EQUAL(30.0, zoomer->zoomRect().left());
    CPPUNIT_ASSERT_EQUAL(60.0, zoomer->zoomRect().right());
  }

  void SetHistograms_DropsZoomStepsOutsideNewData()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });

    auto* zoomer = Zoomer(widget);
    zoomer->zoom(QRectF(60.0, 0.0, 20.0, 3.0));
    const QRectF currentZoom(10.0, 0.0, 20.0, 3.0);
    zoomer->zoom(currentZoom);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: two zoom steps expected.", 2u, zoomer->zoomRectIndex());

    // The first step lies entirely outside the new data, the current one inside.
    widget.SetHistograms({ Series("B", QColor(), 0.0, 40.0) });

    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(zoomer->zoomStack().size()));
    CPPUNIT_ASSERT_EQUAL(1u, zoomer->zoomRectIndex());
    AssertRect(currentZoom, zoomer->zoomRect());
  }

  void SetHistograms_EmptyDisablesControls()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });
    CPPUNIT_ASSERT_MESSAGE("Precondition failed: the widget is not enabled.", widget.isEnabled());

    widget.SetHistograms({});

    CPPUNIT_ASSERT(SeriesItems(widget).isEmpty());
    CPPUNIT_ASSERT(!widget.isEnabled());
  }

  void SetHistograms_EmptyKeepsZoomForNextSeries()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });

    auto* zoomer = Zoomer(widget);
    const QRectF zoomRect(20.0, 0.0, 30.0, 3.0);
    zoomer->zoom(zoomRect);

    widget.SetHistograms({});
    widget.SetHistograms({ Series("B", QColor(), 0.0, 100.0) });

    CPPUNIT_ASSERT_EQUAL(1u, zoomer->zoomRectIndex());
    AssertRect(zoomRect, zoomer->zoomRect());
  }

  void Reset_RemovesSeriesAndZoom()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });

    auto* zoomer = Zoomer(widget);
    zoomer->zoom(QRectF(20.0, 0.0, 30.0, 3.0));

    widget.Reset();

    CPPUNIT_ASSERT(SeriesItems(widget).isEmpty());
    CPPUNIT_ASSERT_EQUAL(0u, zoomer->zoomRectIndex());
    CPPUNIT_ASSERT(!widget.isEnabled());
  }

  void Zoom_NeverBelowZero()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });

    auto* zoomer = Zoomer(widget);

    // A rectangle reaching below zero is cut off at zero.
    zoomer->zoom(QRectF(20.0, -2.0, 30.0, 5.0));
    CPPUNIT_ASSERT_EQUAL(1u, zoomer->zoomRectIndex());
    CPPUNIT_ASSERT_EQUAL(0.0, zoomer->zoomRect().top());
    CPPUNIT_ASSERT_EQUAL(3.0, zoomer->zoomRect().bottom());
    CPPUNIT_ASSERT_EQUAL(20.0, zoomer->zoomRect().left());

    // A rectangle entirely below zero is ignored.
    zoomer->zoom(QRectF(20.0, -5.0, 30.0, 3.0));
    CPPUNIT_ASSERT_EQUAL(1u, zoomer->zoomRectIndex());
  }

  void Zoom_ResetAfterPanReturnsToFullRange()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });

    auto* zoomer = Zoomer(widget);
    PanTo(widget, 30.0, 60.0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: panning must not touch the zoom stack.", 0u, zoomer->zoomRectIndex());

    // "Reset" and Ctrl+right click zoom to the base, which the stack index already is.
    zoomer->zoom(0);

    auto* plot = widget.findChild<QwtPlot*>();
    CPPUNIT_ASSERT_EQUAL(zoomer->zoomBase().left(), plot->axisScaleDiv(QwtPlot::xBottom).lowerBound());
    CPPUNIT_ASSERT_EQUAL(zoomer->zoomBase().right(), plot->axisScaleDiv(QwtPlot::xBottom).upperBound());
    CPPUNIT_ASSERT_EQUAL(zoomer->zoomBase().left(), SpinBoxValue(widget, "doubleSpinBoxMinValue"));
  }

  void ZoomRangeFields_FollowViewBeyondData()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });

    // Wheel zoom and panning can show more than the data range.
    PanTo(widget, -50.0, 150.0);

    CPPUNIT_ASSERT_EQUAL(-50.0, SpinBoxValue(widget, "doubleSpinBoxMinValue"));
    CPPUNIT_ASSERT_EQUAL(150.0, SpinBoxValue(widget, "doubleSpinBoxMaxValue"));
  }

  void ZoomRangeEdited_KeepsPannedFrequencyRange()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("A", QColor(), 0.0, 100.0) });

    // A vertical pan is not on the zoom stack, but editing the gray value range
    // must keep it anyway.
    PanTo(widget, 1.0, 4.0, QwtPlot::yLeft);

    auto* minBox = widget.findChild<QDoubleSpinBox*>("doubleSpinBoxMinValue");
    CPPUNIT_ASSERT_MESSAGE("Spin box not found.", nullptr != minBox);
    minBox->setValue(20.0);
    Q_EMIT minBox->editingFinished();

    auto* zoomer = Zoomer(widget);
    CPPUNIT_ASSERT_EQUAL(1u, zoomer->zoomRectIndex());
    CPPUNIT_ASSERT_EQUAL(20.0, zoomer->zoomRect().left());
    CPPUNIT_ASSERT_EQUAL(1.0, zoomer->zoomRect().top());
    CPPUNIT_ASSERT_EQUAL(4.0, zoomer->zoomRect().bottom());
  }

  void Clipboard_OneBlockPerSeries()
  {
    QmitkHistogramVisualizationWidget widget;
    widget.SetHistograms({ Series("First", QColor(), 0.0, 10.0), Series("Second", QColor(), 0.0, 10.0, 7.0) });

    auto* button = widget.findChild<QPushButton*>("buttonCopyHistogramToClipboard");
    CPPUNIT_ASSERT_MESSAGE("The copy button was not found.", nullptr != button);
    button->click();

    const auto text = QApplication::clipboard()->text().toStdString();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), text.find("First\nMeasurement"));
    CPPUNIT_ASSERT(text.find("\n\nSecond\nMeasurement") != std::string::npos);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkHistogramVisualizationWidget)
