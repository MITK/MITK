/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPlotDialog_h
#define QmitkPlotDialog_h

#include <MitkQtWidgetsExtExports.h>
#include <mitkCommon.h>
#include <QmitkPlotWidget.h>
#include <QDialog>

/**
 * \brief Dialog providing a GUI interface for plotting curves using QmitkPlotWidget.
 *
 * Wraps a QmitkPlotWidget in a QDialog with a "close" button. Usage example:
 * \code
 *   QmitkPlotDialog* dlg = new QmitkPlotDialog("My dialog title", this);
 *   dlg->GetPlot()->SetAxisTitle(QwtPlot::xBottom, "X [mm]");
 *   dlg->GetPlot()->SetAxisTitle(QwtPlot::yLeft, "Y [mm]");
 *   int curveId = dlg->GetPlot()->InsertCurve("My data");
 *   dlg->GetPlot()->SetCurveData(curveId, xValues, yValues);
 *   dlg->GetPlot()->Replot();
 *   dlg->exec();
 * \endcode
 *
 * \sa QmitkPlotWidget, QwtPlot
 */
class MITKQTWIDGETSEXT_EXPORT QmitkPlotDialog : public QDialog
{
public:
  /**
   * \brief Construct the plot dialog.
   * \param[in] title The dialog window title.
   * \param[in] parent The parent widget.
   * \param[in] name Object name (unused, kept for compatibility).
   */
  QmitkPlotDialog(const char *title, QWidget *parent = nullptr, const char *name = nullptr);

  /** \brief Destructor. */
  ~QmitkPlotDialog() override;

  /**
   * \brief Get the embedded QmitkPlotWidget for configuring curves and axes.
   * \return Pointer to the QmitkPlotWidget.
   */
  QmitkPlotWidget *GetPlot();

  /**
   * \brief Get the underlying QwtPlot instance for low-level plot customization.
   * \return Pointer to the QwtPlot.
   */
  QwtPlot *GetQwtPlot();

protected:
  QmitkPlotWidget *m_Plot;

  QPushButton *m_CloseDialogButton;
};

#endif
