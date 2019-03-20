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

#ifndef _QmitkPlotWidget_H_
#define _QmitkPlotWidget_H_

#include "MitkQtWidgetsExtExports.h"
#include "mitkCommon.h"
#include <qwidget.h>
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_symbol.h>
#include <tuple>
#include <vector>

/**
* Provides a convenient interface for plotting curves using qwt.
* Designed for qwt version 5.2.1.
* Can be used with a QmitkPlotDialog, which provides a "Close" button.
* @see QmitkPlotDialog
*
* To plot data do the following:
* 1. Create two QmitkPlotWidget::DataVector Objects and fill them
*    with corresponding x/y values. DataVectors are simple stl-vectors
*    of type std::vector<double>. Please note that the xValues
*    vector and the yValues vector MUST have the same size.
* 2. Instantiate the widget for example like that:
*      QmitkPlotWidget* widget = new QmitkPlotWidget( this, "widget" );
*      widget->SetAxisTitle( QwtPlot::xBottom, "My x asis [mm]" );
*      widget->SetAxisTitle( QwtPlot::yLeft, "My y axis [mm]" );
*      int curveId = widget->InsertCurve( "My sophisticated data" );
*      widget->SetCurveData( curveId, xValues, yValues );
*      widget->SetCurvePen( curveId, QPen( red ) );
*      widget->SetCurveTitle( curveId, "My curve description" );
*      widget->Replot();
* 3. You can modify the behavior of the plot by directly referencing
*    the QwtPlot instance using the method GetPlot().
* @see QwtPlot
* @deprecatedSince{2018_04} Use QmitkChartWidget instead
*/

class MITKQTWIDGETSEXT_EXPORT QmitkPlotWidget
  : public QWidget
  {
private:
  Q_OBJECT

public:
  /**
  * represents the data type used for scalar values stored
  * in data arrays. This type is provided by qwt and may not
  * be changed.
  */
  typedef double ScalarType;

  /**
  * This type may be used to store a set of scalar values
  * representing either x or y coordinates of the data
  * points that should be rendered.
  */
  typedef std::vector<ScalarType> DataVector;

  /**
  * convenience type used to store pairs representing x/y coordinates
  * that should be rendered as a curve by the plot widget
  */
  typedef std::vector<std::pair<double, double>> XYDataVector;

  /**
  * Standard qt constructor
  */
  QmitkPlotWidget(QWidget *parent = nullptr,
                  const char *title = nullptr,
                  const char *name = nullptr,
                  Qt::WindowFlags f = nullptr);

  /**
  * Virtual destructor
  */
  ~QmitkPlotWidget() override;

  /**
  * Returns the instance of the plot-widget. This may be used
  * to modify any detail of the appearance of the plot.
  */
  QwtPlot *GetPlot();

  /**
    * Set the title using (formatted) QwtText object
    */
  void SetPlotTitle(const QwtText &qwt_title);

  /**
    * Set plain text title, using default formatting
    */
  void SetPlotTitle(const char *title);

  /**
  * Inserts a new curve into the plot-window.
  * @param title the name of the curve
  * @returns the id of the curve. Use this id to
  * refer to the curve, if you want to modify or add data.
  */
  unsigned int InsertCurve(const char *title, QColor color = QColor(Qt::black));

  /**
  * Sets the title of the given axis. For the set of available axes
  * @see QwtPlot::Axis.
  * @param axis the axis for which the description should be set.
  * @param title the name of the axis.
  */
  void SetAxisTitle(int axis, const char *title);

  /**
  * Sets the data for a previously added curve. Data is provided as two vectors of double.
  * The first vector represents the x coordinates, the second vector represents the y coordinates.
  * @param curveId the id of the curve for which data should be added.
  * @param xValues the x coordinates of the points that define the curve
  * @param yValues the y coordinates of the points that define the curve
  * @returns whether data was added successfully or not
  */
  bool SetCurveData(unsigned int curveId, const DataVector &xValues, const DataVector &yValues);

  /**
  * @brief Sets the data with errors for a previously added curve.
  *
  * @param curveId the id of the curve for which data should be added.
  * @param xValues the x coordinates of the points that define the curve
  * @param yValues the y coordinates of the points that define the curve
  * @param yLowerError the magnitude (>0) of the error in the lesser direction of y
  * @param yUpperError the magnitude (>0) of the error in the larger direction of y
  * @returns whether data was added successfully or not
  */
  bool SetCurveData(unsigned int curveId,
                    const DataVector &xValues,
                    const DataVector &yValues,
                    const DataVector &yLowerError,
                    const DataVector &yUpperError);

  /**
  * @brief Sets the data with errors for a previously added curve.
  *
  * @param curveId the id of the curve for which data should be added.
  * @param xValues the x coordinates of the points that define the curve
  * @param yValues the y coordinates of the points that define the curve
  * @param xLowerError the magnitude (>0) of the error in the lesser direction of x
  * @param xUpperError the magnitude (>0) of the error in the larger direction of x
  * @param yLowerError the magnitude (>0) of the error in the lesser direction of y
  * @param yUpperError the magnitude (>0) of the error in the larger direction of y
  * @returns whether data was added successfully or not
  */
  bool SetCurveData(unsigned int curveId,
                    const DataVector &xValues,
                    const DataVector &yValues,
                    const DataVector &xLowerError,
                    const DataVector &xUpperError,
                    const DataVector &yLowerError,
                    const DataVector &yUpperError);

  /**
  * Sets the data for a previously added curve. Data is provided as a vectors of pairs.
  * The pairs represent x/y coordinates of the points that define the curve.
  * @param curveId the id of the curve for which data should be added.
  * @param data the coordinates of the points that define the curve
  * @returns whether data was added successfully or not
  */
  bool SetCurveData(unsigned int curveId, const XYDataVector &data);

  /**
  * Defines how a curve should be drawn. For drawing a curve, a QPen is used.
  * @param curveId the id of the curve for which appearance should be changed
  * @param pen a QPen (@see QPen) defining the line style
  */
  void SetCurvePen(unsigned int curveId, const QPen &pen);

  /**
  * Assign a brush, which defines the fill pattern of shapes drawn by a QPainter.
  * In case of brush.style() != QBrush::NoBrush and * style() != QwtPlotCurve::Sticks
  * the area between the curve and the baseline will be filled.
  * In case !brush.color().isValid() the area will be filled by pen.color().
  * The fill algorithm simply connects the first and the last curve point to the
  * baseline. So the curve data has to be sorted (ascending or descending).
  * @param curveId the id of the curve for which appearance should be changed
  * @param brush a QBrush (@see QBrush) defining the line style
  */
  void SetCurveBrush(unsigned int curveId, const QBrush &brush);

  /**
  * Sets the style how the line is drawn for the curve; like, plain line,
  * or with the data points marked with a symbol;
  * @param: style A QwtPlotCurve::CurveStyle
  */
  void SetCurveStyle(unsigned int curveId, const QwtPlotCurve::CurveStyle style);

  /**
  * Sets the style data points are drawn for the curve; like, a line,
  * or dots;
  * @param: symbol A QwtSymbol
  */
  void SetCurveSymbol(unsigned int curveId, QwtSymbol *symbol);

  void SetCurveAntialiasingOn(unsigned int curveId);
  void SetCurveAntialiasingOff(unsigned int curveId);

  /**
  * Sets the title of the given curve. The title will be shown in the legend of
  * the QwtPlot.
  * @param curveId the id of the curve for which the title should be set
  * @param title the description of the curve that will be shown in the legend.
  */
  void SetCurveTitle(unsigned int curveId, const char *title);

  /**
  * Defines how a curves errors should be drawn. For drawing a QPen is used.
  * @param curveId the id of the curve for which error appearance should be changed
  * @param pen a QPen (@see QPen) defining the line style
  */
  void SetErrorPen(unsigned int curveId, const QPen &pen);

  /**
  * Defines the style of errors, symbols or as a curve.
  * @param curveId the id of the curve for which error appearance should be changed
  * @param drawSmybols true - draw symbols, false - draw curve
  */
  void SetErrorStyleSymbols(unsigned int curveId, bool drawSmybols);

  /**
  * Sets the legend of the plot
  *
  */
  void SetLegend(QwtLegend *legend, QwtPlot::LegendPosition pos = QwtPlot::RightLegend, double ratio = -1);

  /**
  * Set a curve's legend attribute
  * @param curveId the id of the curve
  * @param attribute the legend attribute to be set
  */
  void SetLegendAttribute(unsigned int curveId, const QwtPlotCurve::LegendAttribute &attribute);

  /**
  * Triggers a replot of the curve. Replot should be called once after
  * setting new data.
  */
  void Replot();

  /**
  * Resets the plot into an empty state
  */
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
  * @param xValues Vector of x values an error bar belongs to
  * @param values The original data value
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
