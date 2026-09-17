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
 * Nothing is edited by clicking until an edit target is named: until then the
 * base class's per-point editing is suppressed, and the opacity curve is moved
 * as a whole through SetOpacityShift and SetOpacityHeight, which the owner
 * drives from its own controls. Naming a target gives that one function
 * grabbable handles and hands the mouse and keyboard to the base class; the
 * other function stays on show as context.
 *
 * \sa QmitkPiecewiseFunctionCanvas, QmitkColorTransferFunctionCanvas
 */
class MITKQTWIDGETSEXT_EXPORT QmitkCombinedTransferFunctionCanvas : public QmitkPiecewiseFunctionCanvas
{
  Q_OBJECT

  public:
    /** \brief What the mouse edits on this canvas, if anything. */
    enum class EditTarget
    {
      None,    /**< Display only. */
      Opacity, /**< The control points of the opacity curve. */
      Color    /**< The colour points along the bottom edge. */
    };

    QmitkCombinedTransferFunctionCanvas(QWidget *parent = nullptr, Qt::WindowFlags f = {});

    /**
     * \brief Choose which of the two functions the mouse and keyboard act on.
     *
     * Both are drawn whatever the target is; what changes is which one carries
     * handles and answers to a click.
     */
    void SetEditTarget(EditTarget target);
    EditTarget GetEditTarget() const;

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
    void mouseDoubleClickEvent(QMouseEvent *mouseEvent) override;
    void keyPressEvent(QKeyEvent *keyEvent) override;

    /**
     * \brief The point accessors the base class edits through, answered for
     *        whichever function the current target names.
     *
     * The base drives every gesture through these, so overriding them is what
     * lets one canvas edit two functions. The colour function is not a
     * QmitkColorTransferFunctionCanvas here - a widget has one base class, and
     * this one's is the opacity canvas - so the colour half is spelled out.
     */
    int GetNearHandle(int x, int y, unsigned int maxSquaredDistance = 100) override;
    int AddFunctionPoint(double x, double val) override;
    void RemoveFunctionPoint(double x) override;
    void MoveFunctionPoint(int index, std::pair<double, double> pos) override;
    double GetFunctionX(int index) override;
    double GetFunctionY(int index) override;
    int GetFunctionSize() override;
    void DoubleClickOnHandle(int handle) override;

  signals:
    /** \brief Emitted after SetOpacityShift or SetOpacityHeight rebuilt the
     *         scalar-opacity curve. The curve is already updated; deciding
     *         whether anything needs re-rendering is left to the receiver.
     */
    void OpacityChanged();

    /**
     * \brief Emitted after a point was added, moved, removed or recoloured.
     *
     * The function is already updated and the render already requested; this
     * says that the curve on show is no longer the one the owner handed over.
     */
    void PointsChanged();

  private:
    void PaintColorGradient(QPainter &painter);

    /** \brief Draw grabbable handles for the points of the current target. */
    void PaintHandles(QPainter &painter);

    void RebuildOpacityFromBaseline();

    vtkColorTransferFunction *m_ColorTransferFunction;

    EditTarget m_EditTarget { EditTarget::None };

    std::vector<std::pair<double,double>> m_OpacityBasePoints;
    double m_OpacityShift;
    double m_OpacityHeight;
};

#endif
