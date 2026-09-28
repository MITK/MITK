/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <QmitkCombinedTransferFunctionCanvas.h>

#include <mitkRenderingManager.h>

#include <QColorDialog>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPolygon>
#include <QToolTip>

#include <algorithm>

namespace
{
  /** \brief Room below the plot for the color stop markers. */
  constexpr int RAIL_HEIGHT = 20;

  constexpr int ROOF_HEIGHT = 5;
  constexpr int MARKER_WIDTH = 15;

  /** \brief Radius of an opacity handle as drawn.
   *
   * Also how close a press on the rail has to come to one to grab it: handles
   * are drawn over the markers, so a press there grabs whichever it looks to
   * land on.
   */
  constexpr int HANDLE_RADIUS = 4;

  /** \brief Room beside the plot for the outer half of a marker, plus the frame.
   *
   * A stop at either end of the axis is drawn on the end of the axis, so without
   * this half of its marker would fall outside the widget and be clipped away.
   */
  constexpr int SIDE_MARGIN = 1 + MARKER_WIDTH / 2;

  /** \brief The gray below which a stop's own color is too dark for a black
   *         dot to show on it.
   */
  constexpr int DARK_MARKER_GRAY = 128;

  /** \brief Opacity of the light ring just outside a marker's outline. */
  constexpr int MARKER_HALO_ALPHA = 120;

  /** \brief Opacity of a marker standing for a stop the axis does not reach.
   *
   * Faded rather than shaped differently, so that it still carries the color it
   * is there to give hold of, and still reads as one of the markers.
   */
  constexpr int OFF_AXIS_ALPHA = 110;
}

QmitkCombinedTransferFunctionCanvas::QmitkCombinedTransferFunctionCanvas(QWidget *parent, Qt::WindowFlags f)
: QmitkPiecewiseFunctionCanvas(parent, f),
  m_ColorTransferFunction(nullptr),
  m_OpacityShift(0.0),
  m_OpacityHeight(0.0)
{
  // Everything that maps between values and pixels goes through contentsRect(),
  // so reserving the rail and the marker's half-width here is all it takes for
  // the histogram, the gradient and the curve to keep to the plot they leave.
  // Reserved whether or not there is anything to draw in it, so that the canvas
  // does not change size when editing begins.
  this->setContentsMargins(SIDE_MARGIN, 1, SIDE_MARGIN, RAIL_HEIGHT);
}

void QmitkCombinedTransferFunctionCanvas::SetColorTransferFunction(vtkColorTransferFunction *colorTransferFunction)
{
  m_ColorTransferFunction = colorTransferFunction;
  this->update();
}

void QmitkCombinedTransferFunctionCanvas::SetEditable(bool editable)
{
  if (editable == m_Editable)
    return;

  m_Editable = editable;

  // The selection is an index into one of the two functions, so it means nothing
  // once nothing is being edited - and one left over from a previous edit would
  // name a stop this one never selected.
  m_GrabbedHandle = -1;
  m_ActiveFunction = ActiveFunction::Opacity;

  this->update();

  emit ColorStopsChanged();
}

void QmitkCombinedTransferFunctionCanvas::Clear()
{
  m_ColorTransferFunction = nullptr;

  // Owned by the mitk::TransferFunction the caller is dropping, and protected in
  // QmitkPiecewiseFunctionCanvas, whose SetPiecewiseFunction dereferences its
  // argument and so cannot be handed a nullptr.
  m_PiecewiseFunction = nullptr;

  m_OpacityBasePoints.clear();
  m_OpacityShift = 0.0;
  m_OpacityHeight = 0.0;

  // Both functions are gone, so there is nothing left to edit - and nothing for
  // the point accessors to be asked about.
  this->SetEditable(false);

  this->SetHistogram(nullptr);
  this->update();
}

QRect QmitkCombinedTransferFunctionCanvas::ColorStopRail() const
{
  const QRect contents = this->contentsRect();

  return QRect(contents.x(), contents.bottom() + 1, contents.width(), RAIL_HEIGHT);
}

bool QmitkCombinedTransferFunctionCanvas::PressGrabsColorStop(const QPoint &pos)
{
  if (pos.y() <= this->contentsRect().bottom())
    return false;

  const bool onOpacityHandle = m_PiecewiseFunction != nullptr &&
    QmitkPiecewiseFunctionCanvas::GetNearHandle(pos.x(), pos.y(), HANDLE_RADIUS * HANDLE_RADIUS) != -1;

  return !onOpacityHandle;
}

bool QmitkCombinedTransferFunctionCanvas::IsColorStopOffAxis(int index) const
{
  const double value = this->GetColorStopValue(index);

  return value < m_Lower || value > m_Upper;
}

int QmitkCombinedTransferFunctionCanvas::EdgeColorStop(AxisEdge edge) const
{
  const int count = this->GetColorStopCount();
  int nearest = -1;

  // VTK keeps its nodes in order, so the nearest stop below the axis is the last
  // one under it and the nearest above is the first one over it.
  for (int i = 0; i < count; ++i)
  {
    const double value = this->GetColorStopValue(i);

    if (edge == AxisEdge::Lower && value < m_Lower)
      nearest = i;

    if (edge == AxisEdge::Upper && value > m_Upper && nearest == -1)
      nearest = i;
  }

  return nearest;
}

int QmitkCombinedTransferFunctionCanvas::MarkerX(int index)
{
  const QRect contents = this->contentsRect();
  const int halfWidth = MARKER_WIDTH / 2;
  const double value = this->GetColorStopValue(index);

  // Against the outside of the frame, so that the half of it the widget has room
  // for is the half that reads as pointing away from the plot.
  if (value < m_Lower)
    return contents.left() - halfWidth - 1;

  if (value > m_Upper)
    return contents.right() + halfWidth + 1;

  return this->FunctionToCanvas(std::make_pair(value, 0.0)).first;
}

bool QmitkCombinedTransferFunctionCanvas::GrabbedStopIsOffAxis() const
{
  return m_ActiveFunction == ActiveFunction::Color &&
         m_GrabbedHandle != -1 &&
         m_GrabbedHandle < this->GetColorStopCount() &&
         this->IsColorStopOffAxis(m_GrabbedHandle);
}

void QmitkCombinedTransferFunctionCanvas::PaintColorGradient(QPainter &painter)
{
  if (m_ColorTransferFunction == nullptr || !this->isEnabled())
    return;

  const QRect contents = this->contentsRect();
  const int width = contents.width();
  const int height = contents.height();

  if (width <= 0 || m_Upper <= m_Lower)
    return;

  painter.save();

  // One vertical strip per pixel column, colored by the transfer function at that
  // intensity and faded from transparent (top) to semi-opaque (bottom) so the
  // histogram behind it stays visible.
  for (int px = 0; px < width; ++px)
  {
    const double xVal = m_Lower + (static_cast<double>(px) / width) * (m_Upper - m_Lower);

    double rgb[3];
    m_ColorTransferFunction->GetColor(xVal, rgb);
    QColor color(qRound(rgb[0] * 255), qRound(rgb[1] * 255), qRound(rgb[2] * 255));

    QColor transparentTop = color;
    transparentTop.setAlpha(50);
    QColor opaqueBottom = color;
    opaqueBottom.setAlpha(220);

    QLinearGradient fade(0, contents.y(), 0, contents.y() + height);
    fade.setColorAt(0.0, transparentTop);
    fade.setColorAt(1.0, opaqueBottom);

    painter.fillRect(contents.x() + px, contents.y(), 1, height, fade);
  }

  painter.restore();
}

void QmitkCombinedTransferFunctionCanvas::SetOffAxisColorStopToolTip(const QString &toolTip)
{
  m_OffAxisColorStopToolTip = toolTip;
}

bool QmitkCombinedTransferFunctionCanvas::event(QEvent *e)
{
  if (e->type() == QEvent::ToolTip && m_Editable && !m_OffAxisColorStopToolTip.isEmpty())
  {
    const auto *helpEvent = static_cast<QHelpEvent *>(e);
    const QPoint pos = helpEvent->pos();

    // Asked as a press asks, with the reach GetNearHandle is given by default,
    // so that the text shows over exactly the markers a click would take as the
    // stops beyond the axis rather than over one that sits on its very end.
    const int stop = this->PressGrabsColorStop(pos)
      ? this->ColorStopNear(pos.x(), 100)
      : -1;

    if (stop != -1 && this->IsColorStopOffAxis(stop))
    {
      const QRect contents = this->contentsRect();
      const QRect rail = this->ColorStopRail();

      // The margin the marker is drawn in, so that the text goes as soon as the
      // cursor moves on to anything else.
      const QRect margin = pos.x() < contents.left()
        ? QRect(0, rail.top(), contents.left(), rail.height())
        : QRect(contents.right() + 1, rail.top(), this->width() - contents.right() - 1, rail.height());

      QToolTip::showText(helpEvent->globalPos(), m_OffAxisColorStopToolTip, this, margin);
      return true;
    }
  }

  return QmitkPiecewiseFunctionCanvas::event(e);
}

void QmitkCombinedTransferFunctionCanvas::paintEvent(QPaintEvent * /*e*/)
{
  QPainter painter(this);

  // No image/preset selected, or volume rendering is off -> nothing to show.
  // Draw just the empty frame so the histogram and the curve clear instead of
  // lingering from the previous node
  const bool hasContent = this->isEnabled() && m_PiecewiseFunction != nullptr;

  if (hasContent)
  {
    // Back to front: histogram, color gradient, then the opacity curve.
    this->PaintHistogram(painter);
    this->PaintColorGradient(painter);
  }

  const QRect contents = this->contentsRect();
  painter.setPen(Qt::gray);
  painter.drawRect(contents.x() - 1, contents.y() - 1, contents.width() + 1, contents.height() + 1);

  if (!hasContent)
    return;

  // The opacity curve. Handles for its points come last, and only while there
  // is an edit to make with them.
  if (m_PiecewiseFunction->GetSize() > 0)
  {
    double *dp = m_PiecewiseFunction->GetDataPointer();
    const int size = m_PiecewiseFunction->GetSize();

    const double firstX = dp[0];
    const double lastX = dp[(size-1) *2];

    // Build the line: flat 0 up to the first point, through the control points,
    // then drop back to 0 at the last point (instead of running flat off the edges).
    QPolygon curve;
    auto addPoint = [&](double x, double y) {
      const auto c = this->FunctionToCanvas(std::make_pair (x, y));
      curve << QPoint(c.first, c.second);
    };

    // Check if first control Point is above lower display bound, so inside visible axis
    if (m_Lower < firstX)
      addPoint(m_Lower, 0.0);

    addPoint(firstX, 0.0);

    for (int i = 0; i < size; ++i)
    {
      addPoint(dp[i * 2], dp[i * 2 + 1]);
    }

    addPoint(lastX, 0.0);

    // Check if first control Point is below upper display bound, so inside visible axis
    if (m_Upper > lastX)
      addPoint(m_Upper, 0.0);

    // Control points can lie beyond either end of the axis, and the line to them
    // would run on into the margins that belong to the color markers. Clipped at
    // the sides only: zero opacity maps onto the frame's bottom edge, so a clip
    // to the contents would shave off the baseline.
    painter.save();
    painter.setClipRect(QRect(contents.left() - 1, 0, contents.width() + 2, this->height()));

    painter.setPen(QPen(Qt::black, 2));
    painter.drawPolyline(curve);

    painter.restore();
  }

  this->PaintHandles(painter);
}

void QmitkCombinedTransferFunctionCanvas::PaintHandles(QPainter &painter)
{
  if (!m_Editable)
    return;

  // Both functions carry handles at once: which one a gesture means follows from
  // where it lands, so hiding either would only hide what can be done.
  painter.save();

  const int selected = this->GetSelectedColorStop();
  const int lowerEdge = this->EdgeColorStop(AxisEdge::Lower);
  const int upperEdge = this->EdgeColorStop(AxisEdge::Upper);

  // Every stop beyond an end of the axis is drawn on that end, so only the one
  // nearest it is: the rest would land underneath and say nothing. The selected
  // one is always drawn, so that picking one from the list beside the canvas
  // shows here too.
  const auto isShown = [&](int index)
  {
    return index == selected || index == lowerEdge || index == upperEdge ||
           !this->IsColorStopOffAxis(index);
  };

  // Kept to the markers: the antialiasing they need would blur the one-pixel
  // outlines of the handles.
  painter.save();

  // The selected one last, since markers are wide enough that two close stops
  // overlap and the one being worked on is the one that has to stay whole.
  for (int i = 0; i < this->GetColorStopCount(); ++i)
  {
    if (i != selected && isShown(i))
      this->PaintColorStop(painter, i, false);
  }

  if (selected != -1)
    this->PaintColorStop(painter, selected, true);

  painter.restore();

  // Over the markers, since a point at low opacity reaches down onto the rail
  // and is the one a press there grabs.
  if (m_PiecewiseFunction != nullptr)
  {
    double *dp = m_PiecewiseFunction->GetDataPointer();

    for (int i = 0; i < m_PiecewiseFunction->GetSize(); ++i)
    {
      const bool grabbed = m_ActiveFunction == ActiveFunction::Opacity && i == m_GrabbedHandle;
      const auto handle = this->FunctionToCanvas(std::make_pair(dp[i * 2], dp[i * 2 + 1]));

      painter.setPen(Qt::black);
      painter.setBrush(grabbed ? Qt::red : Qt::white);
      painter.drawEllipse(handle.first - HANDLE_RADIUS, handle.second - HANDLE_RADIUS,
                          2 * HANDLE_RADIUS, 2 * HANDLE_RADIUS);
    }
  }

  painter.restore();
}

void QmitkCombinedTransferFunctionCanvas::PaintColorStop(QPainter &painter, int index, bool selected)
{
  const QRect contents = this->contentsRect();
  const QRect rail = this->ColorStopRail();

  const int x = this->MarkerX(index);
  const bool offAxis = this->IsColorStopOffAxis(index);

  const int halfWidth = MARKER_WIDTH / 2;
  const int apexY = rail.top();
  const int eavesY = apexY + ROOF_HEIGHT;
  const int baseY = rail.bottom() - 1;

  // Half a pixel in: a one-pixel pen straddles the path it is given, so an
  // outline on whole coordinates lands half on each neighboring pixel and comes
  // out gray instead of black.
  QPolygonF marker;
  marker << QPointF(x + 0.5, apexY + 0.5)
         << QPointF(x + halfWidth + 0.5, eavesY + 0.5)
         << QPointF(x + halfWidth + 0.5, baseY + 0.5)
         << QPointF(x - halfWidth + 0.5, baseY + 0.5)
         << QPointF(x - halfWidth + 0.5, eavesY + 0.5);

  QColor color = this->GetColorStopColor(index);

  // Drawn on the edge rather than on a value, so it is faded to say that it
  // stands for a stop out beyond the axis rather than naming a place on it.
  if (offAxis)
    color.setAlpha(OFF_AXIS_ALPHA);

  // Without this the roof comes out as a blunt stub: its tip is one pixel wide
  // and the outline alone is as thick.
  painter.setRenderHint(QPainter::Antialiasing);

  // A faint light ring laid down first, of which only the pixel outside the
  // outline survives the fill and the outline drawn over it. Enough to lift a
  // dark marker off a dark stretch of gradient without reading as part of the
  // marker itself.
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(QColor(255, 255, 255, MARKER_HALO_ALPHA), 3));
  painter.drawPolygon(marker);

  painter.setBrush(color);
  painter.setPen(QPen(QColor(0, 0, 0, color.alpha()), 1));
  painter.drawPolygon(marker);

  if (!selected)
    return;

  // An edge marker straddles the frame with half of it outside the widget, so
  // the dot goes in the half that is on show.
  const double dotX = offAxis
    ? (x < contents.left() ? x + 0.5 * halfWidth : x - 0.5 * halfWidth)
    : x + 0.5;

  // The dot sits on the stop's own color, so which of black and white shows up
  // is the color's to decide rather than something that can be fixed here.
  // Weighted gray rather than HSL lightness, which calls a saturated orange dark
  // and would put a white dot on it.
  painter.setPen(Qt::NoPen);
  painter.setBrush(qGray(color.rgb()) < DARK_MARKER_GRAY ? Qt::white : Qt::black);
  painter.drawEllipse(QPointF(dotX, 0.5 * (eavesY + baseY)), 2.5, 2.5);
}

void QmitkCombinedTransferFunctionCanvas::mousePressEvent(QMouseEvent *mouseEvent)
{
  // Until editing is on the canvas is a picture, and gating the handlers is what
  // suppresses the base class's per-point editing: the curve is moved as a whole
  // through the owner's sliders instead.
  if (!m_Editable)
    return;

  const int previous = this->GetSelectedColorStop();
  const QPoint pos = mouseEvent->position().toPoint();

  m_ActiveFunction = this->PressGrabsColorStop(pos)
    ? ActiveFunction::Color
    : ActiveFunction::Opacity;

  // A press on nothing only lets go of the selection. Left to the base class it
  // would add a point, which a click meant to deselect did far too easily;
  // adding is the double click's.
  if (this->GetNearHandle(pos.x(), pos.y()) == -1)
  {
    m_GrabbedHandle = -1;
    this->update();
  }
  else
  {
    QmitkPiecewiseFunctionCanvas::mousePressEvent(mouseEvent);
  }

  // Adding and removing announce themselves; this is for a press that only moved
  // the selection, including one that moved it off the stops altogether.
  if (this->GetSelectedColorStop() != previous)
    emit ColorStopsChanged();
}

void QmitkCombinedTransferFunctionCanvas::mouseMoveEvent(QMouseEvent *mouseEvent)
{
  if (!m_Editable)
    return;

  // There is no position under the cursor for a stop that is not on the axis:
  // the base class clamps a move onto it, so a drag begun on an edge marker
  // would fetch its stop in from wherever out there it sits. Moving one is what
  // the offset control and the wider axis are for.
  if (this->GrabbedStopIsOffAxis())
    return;

  QmitkPiecewiseFunctionCanvas::mouseMoveEvent(mouseEvent);
}

void QmitkCombinedTransferFunctionCanvas::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
  if (!m_Editable)
    return;

  QmitkPiecewiseFunctionCanvas::mouseReleaseEvent(mouseEvent);
}

void QmitkCombinedTransferFunctionCanvas::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
  if (!m_Editable)
    return;

  const QPoint pos = mouseEvent->position().toPoint();

  m_ActiveFunction = this->PressGrabsColorStop(pos)
    ? ActiveFunction::Color
    : ActiveFunction::Opacity;

  const int handle = this->GetNearHandle(pos.x(), pos.y());

  if (handle != -1)
  {
    this->DoubleClickOnHandle(handle);
    return;
  }

  // The margins beside the plot are the markers' room rather than part of the
  // axis, so a double click there names no value to add a point at.
  const QRect contents = this->contentsRect();

  if (mouseEvent->button() != Qt::LeftButton || pos.x() < contents.left() || pos.x() > contents.right())
    return;

  const auto [x, value] = this->CanvasToFunction(std::make_pair(pos.x(), pos.y()));

  // Grabbed as a press on a handle would be, so that holding the second click
  // drags the new point into place.
  m_GrabbedHandle = this->AddFunctionPoint(x, std::clamp(value, 0.0, 1.0));

  this->update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkCombinedTransferFunctionCanvas::keyPressEvent(QKeyEvent *keyEvent)
{
  if (!m_Editable)
    return;

  // As with a drag, and for the same reason. Only the movement is refused:
  // deleting a stop still means the same thing wherever it sits.
  const bool moves = keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right ||
                     keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down;

  if (moves && this->GrabbedStopIsOffAxis())
    return;

  QmitkPiecewiseFunctionCanvas::keyPressEvent(keyEvent);
}

int QmitkCombinedTransferFunctionCanvas::GetNearHandle(int x, int y, unsigned int maxSquaredDistance)
{
  if (m_ActiveFunction != ActiveFunction::Color)
    return QmitkPiecewiseFunctionCanvas::GetNearHandle(x, y, maxSquaredDistance);

  return this->ColorStopNear(x, maxSquaredDistance);
}

int QmitkCombinedTransferFunctionCanvas::ColorStopNear(int x, unsigned int maxSquaredDistance)
{
  // A color stop has no height, so only the distance along the axis decides
  // which one a click means. The nearest rather than the first within reach:
  // markers are wide enough to stand side by side and still overlap.
  int nearest = -1;
  unsigned int nearestDistance = maxSquaredDistance;

  for (int i = 0; i < this->GetColorStopCount(); ++i)
  {
    // Passed over rather than measured: a stop beyond the axis is drawn on the
    // edge instead of where it sits, so its own position says nothing about
    // what a click means - and is far enough out to overflow the squaring.
    if (this->IsColorStopOffAxis(i))
      continue;

    const auto handle = this->FunctionToCanvas(std::make_pair(this->GetColorStopValue(i), 0.0));
    const int distance = handle.first - x;
    const auto squaredDistance = static_cast<unsigned int>(distance * distance);

    if (squaredDistance < nearestDistance)
    {
      nearest = i;
      nearestDistance = squaredDistance;
    }
  }

  if (nearest != -1)
    return nearest;

  // Nothing on the axis was within reach, so a gesture past either end of the
  // plot is one on the marker standing for what lies beyond that end. The stops
  // on the axis are answered for first, since a stop sitting on the very end is
  // drawn across the frame and would otherwise be unreachable.
  const QRect contents = this->contentsRect();

  if (x < contents.left())
    return this->EdgeColorStop(AxisEdge::Lower);

  if (x > contents.right())
    return this->EdgeColorStop(AxisEdge::Upper);

  return -1;
}

int QmitkCombinedTransferFunctionCanvas::AddFunctionPoint(double x, double val)
{
  int index = -1;

  if (m_ActiveFunction == ActiveFunction::Color)
  {
    // The new stop takes the color the gradient already has where it lands, so
    // adding one marks a place to recolor rather than changing anything.
    double rgb[3];
    m_ColorTransferFunction->GetColor(x, rgb);
    index = m_ColorTransferFunction->AddRGBPoint(x, rgb[0], rgb[1], rgb[2]);

    // Selecting it here rather than leaving it to the caller is what lets adding
    // one from a button and adding one by clicking the rail end up in the same
    // state, and what keeps this to a single announcement.
    m_GrabbedHandle = index;

    emit PointsChanged();
    emit ColorStopsChanged();

    return index;
  }

  index = QmitkPiecewiseFunctionCanvas::AddFunctionPoint(x, val);

  emit PointsChanged();

  return index;
}

void QmitkCombinedTransferFunctionCanvas::RemoveFunctionPoint(double x)
{
  if (m_ActiveFunction == ActiveFunction::Color)
  {
    // Every way of removing a stop leaves nothing selected, and the
    // announcement below has to say so already: the index would otherwise
    // name the stop that moved into the removed one's place.
    m_GrabbedHandle = -1;

    m_ColorTransferFunction->RemovePoint(x);

    emit PointsChanged();
    emit ColorStopsChanged();

    return;
  }

  QmitkPiecewiseFunctionCanvas::RemoveFunctionPoint(x);

  emit PointsChanged();
}

void QmitkCombinedTransferFunctionCanvas::MoveFunctionPoint(int index, std::pair<double, double> pos)
{
  if (m_ActiveFunction == ActiveFunction::Color)
  {
    // There is no height to move to, and the stop carries its color along:
    // read it, drop the stop, put it back where the drag asks for it.
    const double from = this->GetFunctionX(index);
    const QColor color = this->GetColorStopColor(index);

    m_ColorTransferFunction->RemovePoint(from);
    m_ColorTransferFunction->AddRGBPoint(pos.first, color.redF(), color.greenF(), color.blueF());

    emit PointsChanged();
    emit ColorStopsChanged();

    return;
  }

  QmitkPiecewiseFunctionCanvas::MoveFunctionPoint(index, pos);

  emit PointsChanged();
}

double QmitkCombinedTransferFunctionCanvas::GetFunctionX(int index)
{
  if (m_ActiveFunction == ActiveFunction::Color)
    return this->GetColorStopValue(index);

  return QmitkPiecewiseFunctionCanvas::GetFunctionX(index);
}

double QmitkCombinedTransferFunctionCanvas::GetFunctionY(int index)
{
  // A color stop sits on the axis, which is what puts its marker at the bottom
  // edge without the drawing or the hit test having to say so.
  if (m_ActiveFunction == ActiveFunction::Color)
    return 0.0;

  return QmitkPiecewiseFunctionCanvas::GetFunctionY(index);
}

int QmitkCombinedTransferFunctionCanvas::GetFunctionSize()
{
  if (m_ActiveFunction == ActiveFunction::Color)
    return this->GetColorStopCount();

  return QmitkPiecewiseFunctionCanvas::GetFunctionSize();
}

void QmitkCombinedTransferFunctionCanvas::DoubleClickOnHandle(int handle)
{
  // Opacity is edited by dragging a handle up and down, so a second click has
  // nothing left to ask about - as in the base class.
  if (m_ActiveFunction != ActiveFunction::Color)
    return;

  m_GrabbedHandle = handle;

  const auto picked = QColorDialog::getColor(this->GetColorStopColor(handle), this);

  if (picked.isValid())
    this->SetSelectedColorStopColor(picked);
}

int QmitkCombinedTransferFunctionCanvas::GetColorStopCount() const
{
  return m_ColorTransferFunction != nullptr
    ? m_ColorTransferFunction->GetSize()
    : 0;
}

double QmitkCombinedTransferFunctionCanvas::GetColorStopValue(int index) const
{
  // Four doubles per color stop - position, red, green, blue - against the two
  // of an opacity point.
  return m_ColorTransferFunction->GetDataPointer()[index * 4];
}

QColor QmitkCombinedTransferFunctionCanvas::GetColorStopColor(int index) const
{
  // Read off the node rather than asked of the function: GetColor interpolates,
  // and what it interpolates in is the function's color space, so a stop's own
  // color is the one place that answer must not be arrived at that way.
  const double *stop = m_ColorTransferFunction->GetDataPointer() + index * 4;

  return QColor::fromRgbF(stop[1], stop[2], stop[3]);
}

int QmitkCombinedTransferFunctionCanvas::GetSelectedColorStop() const
{
  if (!m_Editable || m_ActiveFunction != ActiveFunction::Color)
    return -1;

  return m_GrabbedHandle < this->GetColorStopCount() ? m_GrabbedHandle : -1;
}

void QmitkCombinedTransferFunctionCanvas::SetSelectedColorStop(int index)
{
  if (index < 0 || index >= this->GetColorStopCount() || index == this->GetSelectedColorStop())
    return;

  m_ActiveFunction = ActiveFunction::Color;
  m_GrabbedHandle = index;

  this->update();

  emit ColorStopsChanged();
}

void QmitkCombinedTransferFunctionCanvas::SetSelectedColorStopColor(const QColor &color)
{
  const int index = this->GetSelectedColorStop();

  if (index == -1 || !color.isValid())
    return;

  // VTK replaces a node added at a position it already has one at, so the stop
  // keeps both its place and its index.
  m_ColorTransferFunction->AddRGBPoint(
    this->GetColorStopValue(index), color.redF(), color.greenF(), color.blueF());

  this->update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  emit PointsChanged();
  emit ColorStopsChanged();
}

double QmitkCombinedTransferFunctionCanvas::GetColorStopOffset(int index) const
{
  if (m_Max <= m_Min)
    return -1.0;

  return (this->GetColorStopValue(index) - m_Min) / (m_Max - m_Min);
}

void QmitkCombinedTransferFunctionCanvas::SetSelectedColorStopOffset(double offset)
{
  const int index = this->GetSelectedColorStop();

  if (index == -1 || m_Max <= m_Min)
    return;

  const double requested = m_Min + std::clamp(offset, 0.0, 1.0) * (m_Max - m_Min);

  // Where a drag toward that value would leave the stop, so that the two ways of
  // moving one cannot disagree. A selected stop is the grabbed handle of the
  // color function, which is what the clamp measures from.
  const double value = this->ClampGrabbedHandleX(requested);

  // Typically a stop already pressed against a neighbor: moving it onto itself
  // would still count as an edit of the curve.
  if (value == this->GetColorStopValue(index))
    return;

  this->MoveFunctionPoint(index, std::make_pair(value, 0.0));

  this->update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkCombinedTransferFunctionCanvas::RemoveSelectedColorStop()
{
  const int index = this->GetSelectedColorStop();

  // A gradient has to keep a color to be a gradient at all - the same guard the
  // right-click path observes, in QmitkTransferFunctionCanvas::mousePressEvent.
  if (index == -1 || this->GetColorStopCount() < 2)
    return;

  this->RemoveFunctionPoint(this->GetColorStopValue(index));

  this->update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkCombinedTransferFunctionCanvas::SnapshotOpacityBaseline()
{
  m_OpacityShift = 0.0;
  m_OpacityHeight = 0.0;
  m_OpacityBasePoints.clear();

  if (m_PiecewiseFunction == nullptr)
    return;

  double *dp = m_PiecewiseFunction->GetDataPointer();
  for (int i = 0; i < m_PiecewiseFunction->GetSize(); ++i)
  {
    m_OpacityBasePoints.emplace_back(dp[i * 2], dp[i * 2 + 1]);
  }
}

void QmitkCombinedTransferFunctionCanvas::SetOpacityShift(double shift)
{
  m_OpacityShift = shift;
  this->RebuildOpacityFromBaseline();
}

void QmitkCombinedTransferFunctionCanvas::SetOpacityHeight(double height)
{
  m_OpacityHeight = height;
  this->RebuildOpacityFromBaseline();
}

void QmitkCombinedTransferFunctionCanvas::RebuildOpacityFromBaseline()
{
  if (m_PiecewiseFunction == nullptr || m_OpacityBasePoints.empty())
    return;

  m_PiecewiseFunction->RemoveAllPoints();
  for (const auto &[x,y] : m_OpacityBasePoints)
  {
    // Keep fully transparent points transparent; raise/lower the rest, clamped.
    const double newHeight = (y > 0.0) ? std::clamp(y + m_OpacityHeight, 0.0, 1.0) : 0.0;
    m_PiecewiseFunction->AddPoint(x + m_OpacityShift, newHeight);
  }

  this->update();
  emit OpacityChanged();
}
