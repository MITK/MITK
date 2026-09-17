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

#include <algorithm>

QmitkCombinedTransferFunctionCanvas::QmitkCombinedTransferFunctionCanvas(QWidget *parent, Qt::WindowFlags f)
: QmitkPiecewiseFunctionCanvas(parent, f),
  m_ColorTransferFunction(nullptr),
  m_OpacityShift(0.0),
  m_OpacityHeight(0.0)
{
}

void QmitkCombinedTransferFunctionCanvas::SetColorTransferFunction(vtkColorTransferFunction *colorTransferFunction)
{
  m_ColorTransferFunction = colorTransferFunction;
  this->update();
}

void QmitkCombinedTransferFunctionCanvas::SetEditTarget(EditTarget target)
{
  m_EditTarget = target;

  // The grabbed handle is an index into the function being edited, so it means
  // something else - or nothing at all - the moment the target changes.
  m_GrabbedHandle = -1;

  this->update();
}

QmitkCombinedTransferFunctionCanvas::EditTarget QmitkCombinedTransferFunctionCanvas::GetEditTarget() const
{
  return m_EditTarget;
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
  this->SetEditTarget(EditTarget::None);

  this->SetHistogram(nullptr);
  this->update();
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
  if (m_EditTarget == EditTarget::None)
    return;

  painter.save();

  // The point accessors answer for whichever function is being edited, so one
  // loop serves both targets and only the shape drawn differs.
  for (int i = 0; i < this->GetFunctionSize(); ++i)
  {
    const bool grabbed = i == m_GrabbedHandle;
    const auto handle =
      this->FunctionToCanvas(std::make_pair(this->GetFunctionX(i), this->GetFunctionY(i)));

    if (m_EditTarget == EditTarget::Color)
    {
      // Filled with the colour it carries, so the strip reads as a row of
      // swatches; a colour point answers 0 for its height, which is what sets
      // that strip on the bottom edge.
      double rgb[3];
      m_ColorTransferFunction->GetColor(this->GetFunctionX(i), rgb);

      const int width = grabbed ? 13 : 11;
      const int height = grabbed ? 18 : 14;
      const QRect swatch(handle.first - width / 2, handle.second - height, width, height);

      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor::fromRgbF(rgb[0], rgb[1], rgb[2]));
      painter.drawRect(swatch);

      // A swatch shows the colour the gradient already shows behind it, so an
      // outline of any one colour is lost against some of them. Two nested ones
      // cannot be: whichever of the two the background swallows, the other
      // stands against it.
      painter.setBrush(Qt::NoBrush);
      painter.setPen(grabbed ? Qt::red : Qt::white);
      painter.drawRect(swatch);
      painter.setPen(Qt::black);
      painter.drawRect(swatch.adjusted(-1, -1, 1, 1));
    }
    else
    {
      painter.setPen(Qt::black);
      painter.setBrush(grabbed ? Qt::red : Qt::white);
      painter.drawEllipse(handle.first - 4, handle.second - 4, 8, 8);
    }
  }

  painter.restore();
}

void QmitkCombinedTransferFunctionCanvas::mousePressEvent(QMouseEvent *mouseEvent)
{
  // With no target named the canvas is a picture, and gating the handlers is
  // what suppresses the base class's per-point editing: the curve is moved as a
  // whole through the owner's sliders instead.
  if (m_EditTarget == EditTarget::None)
    return;

  QmitkPiecewiseFunctionCanvas::mousePressEvent(mouseEvent);
}

void QmitkCombinedTransferFunctionCanvas::mouseMoveEvent(QMouseEvent *mouseEvent)
{
  if (m_EditTarget == EditTarget::None)
    return;

  QmitkPiecewiseFunctionCanvas::mouseMoveEvent(mouseEvent);
}

void QmitkCombinedTransferFunctionCanvas::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
  if (m_EditTarget == EditTarget::None)
    return;

  QmitkPiecewiseFunctionCanvas::mouseReleaseEvent(mouseEvent);
}

void QmitkCombinedTransferFunctionCanvas::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
  if (m_EditTarget == EditTarget::None)
    return;

  QmitkPiecewiseFunctionCanvas::mouseDoubleClickEvent(mouseEvent);
}

void QmitkCombinedTransferFunctionCanvas::keyPressEvent(QKeyEvent *keyEvent)
{
  // The base clamps through ValidateCoord, which reads the histogram without
  // checking that there is one, and this canvas is shown for an image whose
  // histogram failed to compute too. Dragging clamps against the axis instead,
  // so only the keyboard has to stand down for such an image.
  if (m_EditTarget == EditTarget::None || this->GetHistogram() == nullptr)
    return;

  QmitkPiecewiseFunctionCanvas::keyPressEvent(keyEvent);
}

int QmitkCombinedTransferFunctionCanvas::GetNearHandle(int x, int y, unsigned int maxSquaredDistance)
{
  if (m_EditTarget != EditTarget::Color)
    return QmitkPiecewiseFunctionCanvas::GetNearHandle(x, y, maxSquaredDistance);

  // A colour point has no height, so only the distance along the axis decides
  // which one a click means - anywhere above a handle counts as on it.
  for (int i = 0; i < this->GetFunctionSize(); ++i)
  {
    const auto handle = this->FunctionToCanvas(std::make_pair(this->GetFunctionX(i), 0.0));
    const int distance = handle.first - x;

    if (static_cast<unsigned int>(distance * distance) < maxSquaredDistance)
      return i;
  }

  return -1;
}

int QmitkCombinedTransferFunctionCanvas::AddFunctionPoint(double x, double val)
{
  int index = -1;

  if (m_EditTarget == EditTarget::Color)
  {
    // The new point takes the colour the gradient already has where it lands,
    // so adding one marks a place to recolour rather than changing anything.
    double rgb[3];
    m_ColorTransferFunction->GetColor(x, rgb);
    index = m_ColorTransferFunction->AddRGBPoint(x, rgb[0], rgb[1], rgb[2]);
  }
  else
  {
    index = QmitkPiecewiseFunctionCanvas::AddFunctionPoint(x, val);
  }

  emit PointsChanged();

  return index;
}

void QmitkCombinedTransferFunctionCanvas::RemoveFunctionPoint(double x)
{
  if (m_EditTarget == EditTarget::Color)
  {
    m_ColorTransferFunction->RemovePoint(x);
  }
  else
  {
    QmitkPiecewiseFunctionCanvas::RemoveFunctionPoint(x);
  }

  emit PointsChanged();
}

void QmitkCombinedTransferFunctionCanvas::MoveFunctionPoint(int index, std::pair<double, double> pos)
{
  if (m_EditTarget == EditTarget::Color)
  {
    // There is no height to move to, and the point carries its colour along:
    // read it, drop the point, put it back where the drag asks for it.
    const double from = this->GetFunctionX(index);

    double rgb[3];
    m_ColorTransferFunction->GetColor(from, rgb);
    m_ColorTransferFunction->RemovePoint(from);
    m_ColorTransferFunction->AddRGBPoint(pos.first, rgb[0], rgb[1], rgb[2]);
  }
  else
  {
    QmitkPiecewiseFunctionCanvas::MoveFunctionPoint(index, pos);
  }

  emit PointsChanged();
}

double QmitkCombinedTransferFunctionCanvas::GetFunctionX(int index)
{
  // Four doubles per colour point - position, red, green, blue - against the
  // two of an opacity point.
  if (m_EditTarget == EditTarget::Color)
    return m_ColorTransferFunction->GetDataPointer()[index * 4];

  return QmitkPiecewiseFunctionCanvas::GetFunctionX(index);
}

double QmitkCombinedTransferFunctionCanvas::GetFunctionY(int index)
{
  // A colour point sits on the axis, which is what puts its handle on the
  // bottom edge without the drawing or the hit test having to say so.
  if (m_EditTarget == EditTarget::Color)
    return 0.0;

  return QmitkPiecewiseFunctionCanvas::GetFunctionY(index);
}

int QmitkCombinedTransferFunctionCanvas::GetFunctionSize()
{
  if (m_EditTarget == EditTarget::Color)
    return m_ColorTransferFunction->GetSize();

  return QmitkPiecewiseFunctionCanvas::GetFunctionSize();
}

void QmitkCombinedTransferFunctionCanvas::DoubleClickOnHandle(int handle)
{
  // Opacity is edited by dragging a handle up and down, so a second click has
  // nothing left to ask about - as in the base class.
  if (m_EditTarget != EditTarget::Color)
    return;

  const double x = this->GetFunctionX(handle);

  double rgb[3];
  m_ColorTransferFunction->GetColor(x, rgb);

  const auto picked = QColorDialog::getColor(QColor::fromRgbF(rgb[0], rgb[1], rgb[2]), this);

  if (!picked.isValid())
    return;

  m_ColorTransferFunction->AddRGBPoint(x, picked.redF(), picked.greenF(), picked.blueF());

  this->update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  emit PointsChanged();
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
