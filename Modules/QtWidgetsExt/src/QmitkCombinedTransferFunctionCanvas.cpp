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
#include <QMouseEvent>
#include <QPainter>
#include <QPolygon>

#include <algorithm>

namespace
{
  /** \brief Room below the plot for the colour stop markers. */
  constexpr int RAIL_HEIGHT = 15;

  /** \brief How far a marker's roof reaches up past the frame.
   *
   * What makes it read as pointing at a value on the gradient rather than
   * sitting somewhere underneath it. One short of the roof's own height, so that
   * what overlaps the plot is the roof and what fills the rail is the body.
   */
  constexpr int ROOF_OVERLAP = 4;

  constexpr int ROOF_HEIGHT = 5;
  constexpr int MARKER_WIDTH = 15;

  /** \brief The grey below which a stop's own colour is too dark for a black
   *         dot to show on it.
   */
  constexpr int DARK_MARKER_GREY = 128;

  /** \brief Opacity of the light ring just outside a marker's outline. */
  constexpr int MARKER_HALO_ALPHA = 120;
}

QmitkCombinedTransferFunctionCanvas::QmitkCombinedTransferFunctionCanvas(QWidget *parent, Qt::WindowFlags f)
: QmitkPiecewiseFunctionCanvas(parent, f),
  m_ColorTransferFunction(nullptr),
  m_OpacityShift(0.0),
  m_OpacityHeight(0.0)
{
  // Everything that maps between values and pixels goes through contentsRect(),
  // so reserving the rail here is all it takes for the histogram, the gradient
  // and the curve to keep to the plot above it. Reserved whether or not there is
  // anything to draw in it, so that the canvas does not change height when
  // editing begins.
  this->setContentsMargins(1, 1, 1, RAIL_HEIGHT);
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

bool QmitkCombinedTransferFunctionCanvas::IsOnColorStopRail(int y) const
{
  return y > this->contentsRect().bottom();
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
  painter.drawRect(0, 0, contents.width() + 1, contents.height() + 1);

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

    painter.setPen(QPen(Qt::black, 2));
    painter.drawPolyline(curve);
  }

  this->PaintHandles(painter);
}

void QmitkCombinedTransferFunctionCanvas::PaintHandles(QPainter &painter)
{
  if (!m_Editable)
    return;

  painter.save();

  // Both functions carry handles at once: which one a gesture means follows from
  // where it lands, so hiding either would only hide what can be done.
  if (m_PiecewiseFunction != nullptr)
  {
    double *dp = m_PiecewiseFunction->GetDataPointer();

    for (int i = 0; i < m_PiecewiseFunction->GetSize(); ++i)
    {
      const bool grabbed = m_ActiveFunction == ActiveFunction::Opacity && i == m_GrabbedHandle;
      const auto handle = this->FunctionToCanvas(std::make_pair(dp[i * 2], dp[i * 2 + 1]));

      painter.setPen(Qt::black);
      painter.setBrush(grabbed ? Qt::red : Qt::white);
      painter.drawEllipse(handle.first - 4, handle.second - 4, 8, 8);
    }
  }

  const int selected = this->GetSelectedColorStop();

  // The selected one last, since markers are wide enough that two close stops
  // overlap and the one being worked on is the one that has to stay whole.
  for (int i = 0; i < this->GetColorStopCount(); ++i)
  {
    if (i != selected)
      this->PaintColorStop(painter, i, false);
  }

  if (selected != -1)
    this->PaintColorStop(painter, selected, true);

  painter.restore();
}

void QmitkCombinedTransferFunctionCanvas::PaintColorStop(QPainter &painter, int index, bool selected)
{
  const QRect contents = this->contentsRect();
  const QRect rail = this->ColorStopRail();

  const int x = this->FunctionToCanvas(std::make_pair(this->GetColorStopValue(index), 0.0)).first;

  const int halfWidth = MARKER_WIDTH / 2;
  const int apexY = contents.bottom() - ROOF_OVERLAP;
  const int eavesY = apexY + ROOF_HEIGHT;
  const int baseY = rail.bottom() - 1;

  // Half a pixel in: a one-pixel pen straddles the path it is given, so an
  // outline on whole coordinates lands half on each neighbouring pixel and comes
  // out grey instead of black.
  QPolygonF marker;
  marker << QPointF(x + 0.5, apexY + 0.5)
         << QPointF(x + halfWidth + 0.5, eavesY + 0.5)
         << QPointF(x + halfWidth + 0.5, baseY + 0.5)
         << QPointF(x - halfWidth + 0.5, baseY + 0.5)
         << QPointF(x - halfWidth + 0.5, eavesY + 0.5);

  const QColor color = this->GetColorStopColor(index);

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
  painter.setPen(QPen(Qt::black, 1));
  painter.drawPolygon(marker);

  if (!selected)
    return;

  // The dot sits on the stop's own colour, so which of black and white shows up
  // is the colour's to decide rather than something that can be fixed here.
  // Weighted grey rather than HSL lightness, which calls a saturated orange dark
  // and would put a white dot on it.
  painter.setPen(Qt::NoPen);
  painter.setBrush(qGray(color.rgb()) < DARK_MARKER_GREY ? Qt::white : Qt::black);
  painter.drawEllipse(QPointF(x + 0.5, 0.5 * (eavesY + baseY)), 2.5, 2.5);
}

void QmitkCombinedTransferFunctionCanvas::mousePressEvent(QMouseEvent *mouseEvent)
{
  // Until editing is on the canvas is a picture, and gating the handlers is what
  // suppresses the base class's per-point editing: the curve is moved as a whole
  // through the owner's sliders instead.
  if (!m_Editable)
    return;

  const int previous = this->GetSelectedColorStop();

  m_ActiveFunction = this->IsOnColorStopRail(mouseEvent->position().toPoint().y())
    ? ActiveFunction::Color
    : ActiveFunction::Opacity;

  QmitkPiecewiseFunctionCanvas::mousePressEvent(mouseEvent);

  // Adding and removing announce themselves; this is for a press that only moved
  // the selection, including one that moved it off the stops altogether.
  if (this->GetSelectedColorStop() != previous)
    emit ColorStopsChanged();
}

void QmitkCombinedTransferFunctionCanvas::mouseMoveEvent(QMouseEvent *mouseEvent)
{
  if (!m_Editable)
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

  m_ActiveFunction = this->IsOnColorStopRail(mouseEvent->position().toPoint().y())
    ? ActiveFunction::Color
    : ActiveFunction::Opacity;

  QmitkPiecewiseFunctionCanvas::mouseDoubleClickEvent(mouseEvent);
}

void QmitkCombinedTransferFunctionCanvas::keyPressEvent(QKeyEvent *keyEvent)
{
  // The base clamps through ValidateCoord, which reads the histogram without
  // checking that there is one, and this canvas is shown for an image whose
  // histogram failed to compute too. Dragging clamps against the axis instead,
  // so only the keyboard has to stand down for such an image.
  if (!m_Editable || this->GetHistogram() == nullptr)
    return;

  QmitkPiecewiseFunctionCanvas::keyPressEvent(keyEvent);
}

int QmitkCombinedTransferFunctionCanvas::GetNearHandle(int x, int y, unsigned int maxSquaredDistance)
{
  if (m_ActiveFunction != ActiveFunction::Color)
    return QmitkPiecewiseFunctionCanvas::GetNearHandle(x, y, maxSquaredDistance);

  // A colour stop has no height, so only the distance along the axis decides
  // which one a click means. The nearest rather than the first within reach:
  // markers are wide enough to stand side by side and still overlap.
  int nearest = -1;
  unsigned int nearestDistance = maxSquaredDistance;

  for (int i = 0; i < this->GetFunctionSize(); ++i)
  {
    const auto handle = this->FunctionToCanvas(std::make_pair(this->GetFunctionX(i), 0.0));
    const int distance = handle.first - x;
    const auto squaredDistance = static_cast<unsigned int>(distance * distance);

    if (squaredDistance < nearestDistance)
    {
      nearest = i;
      nearestDistance = squaredDistance;
    }
  }

  return nearest;
}

int QmitkCombinedTransferFunctionCanvas::AddFunctionPoint(double x, double val)
{
  int index = -1;

  if (m_ActiveFunction == ActiveFunction::Color)
  {
    // The new stop takes the colour the gradient already has where it lands, so
    // adding one marks a place to recolour rather than changing anything.
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
    // There is no height to move to, and the stop carries its colour along:
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
  // A colour stop sits on the axis, which is what puts its marker at the bottom
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
  // Four doubles per colour stop - position, red, green, blue - against the two
  // of an opacity point.
  return m_ColorTransferFunction->GetDataPointer()[index * 4];
}

QColor QmitkCombinedTransferFunctionCanvas::GetColorStopColor(int index) const
{
  // Read off the node rather than asked of the function: GetColor interpolates,
  // and what it interpolates in is the function's colour space, so a stop's own
  // colour is the one place that answer must not be arrived at that way.
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

double QmitkCombinedTransferFunctionCanvas::GetSelectedColorStopOffset() const
{
  const int index = this->GetSelectedColorStop();

  if (index == -1 || m_Max <= m_Min)
    return -1.0;

  return (this->GetColorStopValue(index) - m_Min) / (m_Max - m_Min);
}

void QmitkCombinedTransferFunctionCanvas::SetSelectedColorStopOffset(double offset)
{
  const int index = this->GetSelectedColorStop();

  if (index == -1 || m_Max <= m_Min)
    return;

  const double value = m_Min + std::clamp(offset, 0.0, 1.0) * (m_Max - m_Min);

  // The bounds a drag observes: a stop cannot reach its neighbours, since two
  // at one position are one stop as far as VTK is concerned. Refused rather
  // than nudged, so that what the stop did and what the control asked for do
  // not quietly differ.
  const double lower = index > 0 ? this->GetColorStopValue(index - 1) : m_Min;
  const double upper = index < this->GetColorStopCount() - 1 ? this->GetColorStopValue(index + 1) : m_Max;

  if (value <= lower || value >= upper)
    return;

  this->MoveFunctionPoint(index, std::make_pair(value, 0.0));

  this->update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

int QmitkCombinedTransferFunctionCanvas::AddColorStop(double value)
{
  if (m_ColorTransferFunction == nullptr)
    return -1;

  m_ActiveFunction = ActiveFunction::Color;

  const int index = this->AddFunctionPoint(value, 0.0);

  this->update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return index;
}

void QmitkCombinedTransferFunctionCanvas::RemoveSelectedColorStop()
{
  const int index = this->GetSelectedColorStop();

  // A gradient has to keep a colour to be a gradient at all - the same guard the
  // right-click path observes, in QmitkTransferFunctionCanvas::mousePressEvent.
  if (index == -1 || this->GetColorStopCount() < 2)
    return;

  const double value = this->GetColorStopValue(index);

  // Dropped before the removal so that the one announcement it makes already
  // describes the selection as well.
  m_GrabbedHandle = -1;

  this->RemoveFunctionPoint(value);

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
