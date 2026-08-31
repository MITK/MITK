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
 * beneath the curve.
 *
 * Nothing is edited by clicking: the base class's per-point editing is
 * suppressed, and the opacity curve is moved as a whole through
 * SetOpacityShift and SetOpacityHeight, which the owner drives from its own
 * controls. The color function is shown for context only.
 *
 * \sa QmitkPiecewiseFunctionCanvas, QmitkColorTransferFunctionCanvas
 */
class MITKQTWIDGETSEXT_EXPORT QmitkCombinedTransferFunctionCanvas : public QmitkPiecewiseFunctionCanvas
{
  Q_OBJECT

  public:
    QmitkCombinedTransferFunctionCanvas(QWidget *parent = nullptr, Qt::WindowFlags f = {});

    /** \brief Capture the current opacity curve as the baseline the shift/height
     *        offsets apply to, and reset both offsets to 0.
     */
    void SnapshotOpacityBaseline();
    /** \brief Shift the whole opacity curve along the intensity axis
     *         (offset from the baseline captured by SnapshotOpacityBaseline).
     */
    void SetOpacityShift(double shift);
    /** \brief Raise/lower the opacity curve: a signed offset added to every
     *         non-transparent baseline point, clamped to [0, 1].
     */
    void SetOpacityHeight(double height);
    /**
     * \brief Set the color transfer function shown as the background gradient.
     * \param[in] colorTransferFunction The color function to display; may be nullptr.
     */
    void SetColorTransferFunction(vtkColorTransferFunction *colorTransferFunction);

    /**
     * \brief Stop displaying the current functions and histogram.
     *
     * The canvas keeps all three as raw pointers owned elsewhere, so a caller
     * that drops the last reference to them has to say so - otherwise the next
     * paint reads freed memory.
     */
    void Clear();

    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent *mouseEvent) override;

  signals:
    /** \brief Emitted after SetOpacityShift or SetOpacityHeight rebuilt the
     *         scalar-opacity curve. The curve is already updated; deciding
     *         whether anything needs re-rendering is left to the receiver.
     */
    void OpacityChanged();

  private:
    void PaintColorGradient(QPainter &painter);
    void RebuildOpacityFromBaseline();

    vtkColorTransferFunction *m_ColorTransferFunction;

    std::vector<std::pair<double,double>> m_OpacityBasePoints;
    double m_OpacityShift;
    double m_OpacityHeight;
};

#endif
