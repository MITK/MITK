/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPlotWidget_h
#define QmitkPlotWidget_h

#include <MitkQtWidgetsExtExports.h>
#include <mitkCommon.h>
#include <QWidget>
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_symbol.h>
#include <qwt_text.h>
#include <tuple>
#include <vector>

/**
 * \brief Convenience widget providing a high-level interface for plotting curves using Qwt.
 *
 * Wraps a QwtPlot and provides methods for inserting curves, setting data,
 * configuring appearance (pen, brush, symbols), and managing error bars.
 * Can be used standalone or inside a QmitkPlotDialog.
 *
 * Usage example:
 * \code
 *   QmitkPlotWidget* widget = new QmitkPlotWidget(this, "widget");
 *   widget->SetAxisTitle(QwtPlot::xBottom, "X [mm]");
 *   widget->SetAxisTitle(QwtPlot::yLeft, "Y [mm]");
 *   int curveId = widget->InsertCurve("My data");
 *   widget->SetCurveData(curveId, xValues, yValues);
 *   widget->SetCurvePen(curveId, QPen(Qt::red));
 *   widget->Replot();
 * \endcode
 *
 * \deprecated Since 2018.04. Use QmitkChartWidget instead.
 * \sa QmitkPlotDialog, QwtPlot
 */

class MITKQTWIDGETSEXT_EXPORT QmitkPlotWidget
  : public QWidget
  {
private:
  Q_OBJECT

public:
  /** \brief Scalar type used for data values (double, defined by Qwt). */
  typedef double ScalarType;

  /** \brief Vector of scalar values representing x or y coordinates. */
  typedef std::vector<ScalarType> DataVector;

  /** \brief Vector of (x, y) pairs representing curve data points. */
  typedef std::vector<std::pair<double, double>> XYDataVector;

  /**
   * \brief Construct the plot widget.
   * \param[in] parent The parent widget.
   * \param[in] title The plot title.
   * \param[in] name The object name (unused, kept for compatibility).
   * \param[in] f Window flags.
   */
  QmitkPlotWidget(QWidget *parent = nullptr,
                  const char *title = nullptr,
                  const char *name = nullptr,
                  Qt::WindowFlags f = {});

  /** \brief Destructor. Clears all curves and deletes the plot. */
  ~QmitkPlotWidget() override;

  /**
   * \brief Get the underlying QwtPlot instance for low-level customization.
   * \return Pointer to the QwtPlot.
   */
  QwtPlot *GetPlot();

  /**
   * \brief Set the plot title using a formatted QwtText.
   * \param[in] qwt_title The formatted title.
   */
  void SetPlotTitle(const QwtText &qwt_title);

  /**
   * \brief Set the plot title using plain text.
   * \param[in] title The title string.
   */
  void SetPlotTitle(const char *title);

  /**
   * \brief Insert a new curve into the plot.
   * \param[in] title The curve name (shown in legend).
   * \param[in] color The curve title text color (default black).
   * \return The curve ID used to reference this curve in subsequent calls.
   */
  unsigned int InsertCurve(const char *title, QColor color = QColor(Qt::black));

  /**
   * \brief Set the title of an axis.
   * \param[in] axis The axis identifier (e.g. QwtPlot::xBottom, QwtPlot::yLeft).
   * \param[in] title The axis title.
   */
  void SetAxisTitle(int axis, const char *title);

  /**
   * \brief Set curve data from separate x and y vectors.
   * \param[in] curveId The curve ID.
   * \param[in] xValues The x coordinates.
   * \param[in] yValues The y coordinates.
   * \pre xValues and yValues must have the same size.
   * \return True if data was set successfully.
   */
  bool SetCurveData(unsigned int curveId, const DataVector &xValues, const DataVector &yValues);

  /**
   * \brief Set curve data with y-axis error bars.
   * \param[in] curveId The curve ID.
   * \param[in] xValues The x coordinates.
   * \param[in] yValues The y coordinates.
   * \param[in] yLowerError The magnitude of the error in the negative y direction.
   * \param[in] yUpperError The magnitude of the error in the positive y direction.
   * \return True if data was set successfully.
   */
  bool SetCurveData(unsigned int curveId,
                    const DataVector &xValues,
                    const DataVector &yValues,
                    const DataVector &yLowerError,
                    const DataVector &yUpperError);

  /**
   * \brief Set curve data with both x and y error bars.
   * \param[in] curveId The curve ID.
   * \param[in] xValues The x coordinates.
   * \param[in] yValues The y coordinates.
   * \param[in] xLowerError The magnitude of the error in the negative x direction.
   * \param[in] xUpperError The magnitude of the error in the positive x direction.
   * \param[in] yLowerError The magnitude of the error in the negative y direction.
   * \param[in] yUpperError The magnitude of the error in the positive y direction.
   * \return True if data was set successfully.
   */
  bool SetCurveData(unsigned int curveId,
                    const DataVector &xValues,
                    const DataVector &yValues,
                    const DataVector &xLowerError,
                    const DataVector &xUpperError,
                    const DataVector &yLowerError,
                    const DataVector &yUpperError);

  /**
   * \brief Set curve data from a vector of (x, y) pairs.
   * \param[in] curveId The curve ID.
   * \param[in] data The vector of coordinate pairs.
   * \return True if data was set successfully.
   */
  bool SetCurveData(unsigned int curveId, const XYDataVector &data);

  /**
   * \brief Set the pen (line style) for a curve.
   * \param[in] curveId The curve ID.
   * \param[in] pen The QPen defining line color, width, and style.
   */
  void SetCurvePen(unsigned int curveId, const QPen &pen);

  /**
   * \brief Set the brush (fill pattern) for a curve.
   *
   * When set, the area between the curve and the baseline is filled.
   * Curve data should be sorted for correct filling.
   *
   * \param[in] curveId The curve ID.
   * \param[in] brush The QBrush defining the fill pattern.
   */
  void SetCurveBrush(unsigned int curveId, const QBrush &brush);

  /**
   * \brief Set the curve drawing style (lines, sticks, dots, etc.).
   * \param[in] curveId The curve ID.
   * \param[in] style The QwtPlotCurve::CurveStyle.
   */
  void SetCurveStyle(unsigned int curveId, const QwtPlotCurve::CurveStyle style);

  /**
   * \brief Set the symbol used for drawing data points.
   * \param[in] curveId The curve ID.
   * \param[in] symbol The QwtSymbol (ownership transferred to the curve).
   */
  void SetCurveSymbol(unsigned int curveId, QwtSymbol *symbol);

  /**
   * \brief Enable antialiasing for a curve.
   * \param[in] curveId The curve ID.
   */
  void SetCurveAntialiasingOn(unsigned int curveId);

  /**
   * \brief Disable antialiasing for a curve.
   * \param[in] curveId The curve ID.
   */
  void SetCurveAntialiasingOff(unsigned int curveId);

  /**
   * \brief Set the title of a curve (shown in the plot title, not legend).
   * \param[in] curveId The curve ID.
   * \param[in] title The curve title.
   */
  void SetCurveTitle(unsigned int curveId, const char *title);

  /**
   * \brief Set the pen for drawing error bars.
   * \param[in] curveId The curve ID.
   * \param[in] pen The QPen for error bars.
   */
  void SetErrorPen(unsigned int curveId, const QPen &pen);

  /**
   * \brief Set error bar drawing style.
   * \param[in] curveId The curve ID.
   * \param[in] drawSmybols True to draw error bars as symbols, false as a filled tube.
   */
  void SetErrorStyleSymbols(unsigned int curveId, bool drawSmybols);

  /**
   * \brief Set the plot legend.
   * \param[in] legend The QwtLegend widget.
   * \param[in] pos The legend position (default: right).
   * \param[in] ratio The legend size ratio (-1 for automatic).
   */
  void SetLegend(QwtLegend *legend, QwtPlot::LegendPosition pos = QwtPlot::RightLegend, double ratio = -1);

  /**
   * \brief Set a legend attribute for a curve.
   * \param[in] curveId The curve ID.
   * \param[in] attribute The legend attribute to set.
   */
  void SetLegendAttribute(unsigned int curveId, const QwtPlotCurve::LegendAttribute &attribute);

  /** \brief Trigger a replot. Call after setting new data or changing appearance. */
  void Replot();

  /** \brief Clear all curves and reset the plot to an empty state. */
  void Clear();

protected:
  /**
  * Converts the given values into a raw double* array.
  * A new array is allocated via new and must be deleted[] by the caller.
  */
  double *ConvertToRawArray(const DataVector &values);

  /**
  * Converts the given values into a raw double* array.
  * A new array is allocated via new and must be deleted[] by the caller.
  * @param values the x/y values to convert to an array
  * @param component defines if the x values (0) or the y values(1) should
  * be converted. Other values than 0 and 1 will not be accepted.
  */
  double *ConvertToRawArray(const XYDataVector &values, unsigned int component);

  /**
  * Adds an error interval curve.
  *
  * All errors should be absolutes. The magnitude will be used.
  *
  * @param curveId Which curve should the error curve be added to
  * @param lessError Error in the negative direction (value - lessError)
  * @param moreError Error in the positive direction (value + lessError)
  * @param isXError Should the error bars be drawn horizontally
  */
  bool AddErrorIntervalCurve(unsigned int curveId,
                             const DataVector &lessError,
                             const DataVector &moreError,
                             bool isXError);

  QwtPlot *m_Plot;
  std::vector<std::tuple<QwtPlotCurve *, QwtPlotIntervalCurve *, QwtPlotIntervalCurve *>> m_PlotCurveVector;
};

#endif
