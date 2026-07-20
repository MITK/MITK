/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCombinedTransferFunctionCanvas_h
#define QmitkCombinedTransferFunctionCanvas_h

#include <MitkQtWidgetsExtExports.h>
#include <QmitkPiecewiseFunctionCanvas.h>

#include <vtkColorTransferFunction.h>

#include <utility>
#include <vector>

/**
 * \brief Single-canvas transfer function editor combining the histogram, the
 *        color gradient, and the scalar-opacity curve in one view.
 *
 * Extends QmitkPiecewiseFunctionCanvas (opacity curve + histogram background +
 * coordinate transforms) by drawing the color transfer function as a gradient
 * beneath the curve, and by replacing per-point editing with two whole-curve
 * gestures: a horizontal drag shifts the opacity curve along the intensity
 * axis, and Control+drag raises or lowers its overall height. The color
 * function is shown for context only and is not edited here.
 *
 * \sa QmitkPiecewiseFunctionCanvas, QmitkColorTransferFunctionCanvas
 */
class MITKQTWIDGETSEXT_EXPORT QmitkCombinedTransferFunctionCanvas : public QmitkPiecewiseFunctionCanvas
{
  Q_OBJECT

  public:
    QmitkCombinedTransferFunctionCanvas(QWidget *parent = nullptr, Qt::WindowFlags f = {});

    /**
     * \brief Set the color transfer function shown as the background gradient.
     * \param[in] colorTransferFunction The color function to display; may be nullptr.
     */
    void SetColorTransferFunction(vtkColorTransferFunction *colorTransferFunction);

    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent *mouseEvent) override;
  
  signals:
    /** \brief Emitted after a drag has changed the scalar-opacity curve. */
    void OpacityChanged();

  private:
    /** \brief Effect of a mouse drag on the scalar-opacity curve */
    enum class DragMode
    {
      None,
      Shift,
      AdjustHeight
    };

    void PaintColorGradient(QPainter &painter);
    void ApplyDrag(const std::pair<double, double> &functionPos);

    vtkColorTransferFunction *m_ColorTransferFunction;

    DragMode m_DragMode;
    std::pair<double, double> m_DragStart;
    std::vector<std::pair<double, double>> m_DragBasePoints;
};

#endif 
