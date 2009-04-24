#ifndef _QmitkPlotDialog_H
#define _QmitkPlotDialog_H

#include <qdialog.h>
#include <QmitkPlotWidget.h>
#include "mitkCommon.h"

/** 
* Provides a GUI interface for plotting curves using QmitkPlotWidget.
* Designed for qwt version 5.1.1.
* To plot data do the following:
* 1. Create two QmitkPlotDialog::DataVector Objects and fill them
*    with corresponding x/y values. DataVectors are simple stl-vectors
*    of type std::vector<double>. Please note that the xValues
*    vector and the yValues vector MUST have the same size.
* 2. Instantiate and show the dialog for example like that:
*      QmitkPlotDialog* dlg = new QmitkPlotDialog( "My dialog title", this, "dlg" );
*      dlg->GetPlot()->SetAxisTitle( QwtPlot::xBottom, "My x asis [mm]" );
*      dlg->GetPlot()->SetAxisTitle( QwtPlot::yLeft, "My y axis [mm]" );
*      int curveId = dlg->GetPlot()->InsertCurve( "My sophisticated data" );
*      dlg->GetPlot()->SetCurveData( curveId, xValues, yValues );
*      dlg->GetPlot()->SetCurvePen( curveId, QPen( red ) );
*      dlg->GetPlot()->SetCurveTitle( curveId, "My curve description" );
*      dlg->GetPlot()->Replot();
*      dlg->exec();
* 3. You can modify the behavior of the plot by directly referencing
*    the QwtPlot instance using the method GetQwtPlot().
* @see QwtPlot, @see QmitkPlotWidget
*/
class QMITK_EXPORT QmitkPlotDialog: public QDialog 
{
  Q_OBJECT

public:
  /**
  * Standard qt constructor
  */
  QmitkPlotDialog(const char* title, QWidget* parent = 0, const char* name = 0);    

  /**
  * Virtual destructor
  */
  virtual ~QmitkPlotDialog();

  /**
  * Returns the instance of the QmitkPlotWidget. This may be used
  * to modify any detail of the appearance of the plot.
  */
  QmitkPlotWidget* GetPlot();    

  /**
  * Returns the instance of the plot-widget. This may be used
  * to modify any detail of the appearance of the plot.
  */
  QwtPlot* GetQwtPlot();    

protected:

  QmitkPlotWidget* m_Plot;

  QPushButton* m_CloseDialogButton;
};

#endif 

