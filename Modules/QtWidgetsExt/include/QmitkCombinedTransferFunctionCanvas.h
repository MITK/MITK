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

#include <QColor>
#include <QString>

#include <utility>
#include <vector>

/**
 * \brief Single-canvas transfer function editor combining the histogram, the
 *        color gradient, and the scalar-opacity curve in one view.
 *
 * Extends QmitkPiecewiseFunctionCanvas (opacity curve + histogram background +
 * coordinate transforms) by drawing the color transfer function as a gradient
 * beneath the curve, and its control points as markers in a rail along the
 * bottom edge.
 *
 * Nothing is edited by clicking until SetEditable turns editing on: until then
 * the base class's per-point editing is suppressed, and the opacity curve is
 * moved as a whole through SetOpacityShift and SetOpacityHeight, which the owner
 * drives from its own controls.
 *
 * Once editing is on, both functions carry handles at once and the region
 * clicked decides which one a gesture means: the plot belongs to the opacity
 * curve, the rail below it to the colors. A caller therefore has no mode to
 * offer and no mode to keep in step.
 *
 * \sa QmitkPiecewiseFunctionCanvas, QmitkColorTransferFunctionCanvas
 */
class MITKQTWIDGETSEXT_EXPORT QmitkCombinedTransferFunctionCanvas : public QmitkPiecewiseFunctionCanvas
{
  Q_OBJECT

  public:
    QmitkCombinedTransferFunctionCanvas(QWidget *parent = nullptr, Qt::WindowFlags f = {});

    /**
     * \brief Give both functions grabbable handles, or take them away again.
     *
     * Turning editing off also drops the selection, which indexes a function
     * and so means nothing once nothing is being edited.
     */
    void SetEditable(bool editable);

    /** \brief Capture the current opacity curve as the baseline the shift/height
     *         offsets apply to, and reset both offsets to 0.
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

    /**
     * \brief The color stops, for an owner presenting them as a list.
     *
     * Reading them here rather than from the vtkColorTransferFunction directly
     * keeps one place answering for what a stop is and where it sits, which is
     * the same place the gestures write to.
     */
    int GetColorStopCount() const;
    double GetColorStopValue(int index) const;
    QColor GetColorStopColor(int index) const;

    /**
     * \brief Where a stop sits in the displayed range, 0 at its start and 1 at
     *        its end.
     *
     * A fraction rather than the value itself, because the canvas carries no
     * scale for a value to be read against.
     *
     * \return The offset, or -1 where the range is empty.
     */
    double GetColorStopOffset(int index) const;

    /**
     * \brief Whether a stop names a value the axis does not reach.
     *
     * The presets are authored on the scale their modality is measured in, while
     * the axis covers the band the image occupies, so the outermost stops of a
     * preset routinely sit outside it.
     */
    bool IsColorStopOffAxis(int index) const;

    /** \brief The selected color stop, or -1 when the selection is an opacity
     *         point or there is none.
     */
    int GetSelectedColorStop() const;
    void SetSelectedColorStop(int index);

    /** \brief Recolor the selected stop, leaving it where it is. */
    void SetSelectedColorStopColor(const QColor &color);

    /**
     * \brief Move the selected stop to a fraction of the displayed range.
     *
     * Where it would reach or pass a neighbor it stops a pixel short, as a
     * dragged one does, so that the two ways of moving a stop cannot disagree.
     */
    void SetSelectedColorStopOffset(double offset);

    /** \brief Remove the selected stop, unless it is the only one left. */
    void RemoveSelectedColorStop();

    /**
     * \brief The tooltip shown over the marker of a stop the axis does not
     *        reach, while editing.
     *
     * Left to the owner, since how to bring such a stop onto the axis is the
     * owner's to offer. Empty, the default, shows none.
     */
    void SetOffAxisColorStopToolTip(const QString &toolTip);

    bool event(QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent *mouseEvent) override;
    void mouseDoubleClickEvent(QMouseEvent *mouseEvent) override;
    void keyPressEvent(QKeyEvent *keyEvent) override;

    /**
     * \brief The point accessors the base class edits through, answered for
     *        whichever function the gesture in progress concerns.
     *
     * The base drives every gesture through these, so overriding them is what
     * lets one canvas edit two functions. The color function is not a
     * QmitkColorTransferFunctionCanvas here - a widget has one base class, and
     * this one's is the opacity canvas - so the color half is spelled out.
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
     * \brief Emitted after a point was added, moved, removed or recolored.
     *
     * The function is already updated and the render already requested; this
     * says that the curve on show is no longer the one the owner handed over.
     */
    void PointsChanged();

    /**
     * \brief Emitted after the color stops, or which of them is selected,
     *        changed.
     *
     * Separate from PointsChanged because that one also fires for every
     * intermediate position of an opacity drag, which says nothing about the
     * colors and would have an owner rebuilding a list of them per mouse move.
     */
    void ColorStopsChanged();

  private:
    /** \brief Which function the selection, and so the mouse and the keyboard,
     *         acts on.
     *
     * Follows from where a press landed rather than from a mode a caller sets,
     * and then stands for the rest of that gesture and until the next press -
     * which is exactly as long as m_GrabbedHandle indexes that function.
     */
    enum class ActiveFunction
    {
      Opacity,
      Color
    };

    /** \brief An end of the displayed value range, which a stop can lie beyond. */
    enum class AxisEdge
    {
      Lower,
      Upper
    };

    /** \brief The strip along the bottom edge that the color stops sit in. */
    QRect ColorStopRail() const;

    /**
     * \brief Whether a press at this position is meant for the color stops
     *        rather than the opacity points.
     *
     * The rail belongs to the stops and the plot to the points, except where a
     * point at low opacity is drawn over the markers.
     */
    bool PressGrabsColorStop(const QPoint &pos);

    /**
     * \brief The stop standing for everything beyond one end of the axis.
     *
     * The nearest one out there rather than the outermost: it is the stop the
     * visible end of the gradient is interpolated against, and the one a gesture
     * at that end is most likely to mean.
     *
     * \return The index, or -1 where nothing lies beyond that end.
     */
    int EdgeColorStop(AxisEdge edge) const;

    /**
     * \brief The stop a gesture on the rail at this x means, or -1.
     *
     * Apart from GetNearHandle, which answers for whichever function the last
     * press chose, so that a tooltip can ask without choosing one.
     */
    int ColorStopNear(int x, unsigned int maxSquaredDistance);

    /** \brief Where a stop's marker is drawn: on the value it names, or pressed
     *         against the edge it lies beyond.
     */
    int MarkerX(int index);

    /** \brief Whether the gesture in progress has hold of a stop that is not on
     *         the axis, and so has nowhere on the axis to be moved to.
     */
    bool GrabbedStopIsOffAxis() const;

    void PaintColorGradient(QPainter &painter);

    /** \brief Draw grabbable handles for both functions. */
    void PaintHandles(QPainter &painter);

    /**
     * \brief One marker per color stop: a house whose roof points at the value
     *        the color applies to.
     *
     * A stop the axis does not reach is drawn half outside the frame and faded,
     * which says that it lies beyond that end rather than claiming a value on
     * the axis for it.
     */
    void PaintColorStop(QPainter &painter, int index, bool selected);

    void RebuildOpacityFromBaseline();

    vtkColorTransferFunction *m_ColorTransferFunction;

    bool m_Editable { false };
    ActiveFunction m_ActiveFunction { ActiveFunction::Opacity };

    QString m_OffAxisColorStopToolTip;

    std::vector<std::pair<double,double>> m_OpacityBasePoints;
    double m_OpacityShift;
    double m_OpacityHeight;
};

#endif
