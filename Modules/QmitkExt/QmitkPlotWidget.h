#ifndef _QmitkPlotWidget_H_
#define _QmitkPlotWidget_H_

#include <qwidget.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <vector>
#include "mitkCommon.h"

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
*/

class QMITKEXT_EXPORT QmitkPlotWidget: public QWidget 
{

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
  typedef std::vector< std::pair< double, double > > XYDataVector;

  /**
  * Standard qt constructor
  */
  QmitkPlotWidget(QWidget* parent = 0,const char* title = 0, const char* name = 0, Qt::WindowFlags f = 0);    

  /**
  * Virtual destructor
  */
  virtual ~QmitkPlotWidget();

  /**
  * Returns the instance of the plot-widget. This may be used
  * to modify any detail of the appearance of the plot.
  */
  QwtPlot* GetPlot();
  
  void SetPlotTitle(const char* title);

  /**
  * Inserts a new curve into the plot-window. 
  * @param title the name of the curve
  * @returns the id of the curve. Use this id to 
  * refer to the curve, if you want to modify or add data.
  */
  unsigned int InsertCurve( const char* title );

  /**
  * Sets the title of the given axis. For the set of available axes
  * @see QwtPlot::Axis.
  * @param axis the axis for which the description should be set.
  * @param title the name of the axis.
  */
  void SetAxisTitle(int axis, const char* title);

  /**
  * Sets the data for a previously added curve. Data is provided as two vectors of double. 
  * The first vector represents the x coordinates, the second vector represents the y coordinates.
  * @param curveId the id of the curve for which data should be added.
  * @param xValues the x coordinates of the points that define the curve
  * @param yValues the y coordinates of the points that define the curve
  * @returns whether data was added successfully or not
  */
  bool SetCurveData( unsigned int curveId, const DataVector& xValues, const DataVector& yValues );

  /**
  * Sets the data for a previously added curve. Data is provided as a vectors of pairs. 
  * The pairs represent x/y coordinates of the points that define the curve.
  * @param curveId the id of the curve for which data should be added.
  * @param data the coordinates of the points that define the curve
  * @returns whether data was added successfully or not
  */
  bool SetCurveData( unsigned int curveId, const XYDataVector& data );

  /** 
  * Defines how a curve should be drawn. For drawing a curve, a QPen is used.
  * @param curveId the id of the curve for which appearance should be changed
  * @param pen a QPen (@see QPen) defining the line style
  */
  void SetCurvePen( unsigned int curveId, const QPen& pen );

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
  void SetCurveBrush( unsigned int curveId, const QBrush& brush);

  /**
  * Sets the title of the given curve. The title will be shown in the legend of
  * the QwtPlot.
  * @param curveId the id of the curve for which the title should be set
  * @param title the description of the curve that will be shown in the legend.
  */
  void SetCurveTitle( unsigned int curveId, const char* title );

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
  double* ConvertToRawArray( const DataVector& values );

  /** 
  * Converts the given values into a raw double* array. 
  * A new array is allocated via new and must be deleted[] by the caller. 
  * @param values the x/y values to convert to an array
  * @param component defines if the x values (0) or the y values(1) should 
  * be converted. Other values than 0 and 1 will not be accepted.
  */
  double* ConvertToRawArray( const XYDataVector& values, unsigned int component );

  QwtPlot*                    m_Plot;
  std::vector<QwtPlotCurve*>  m_PlotCurveVector;
};

#endif 

