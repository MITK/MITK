/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <QmitkCombinedTransferFunctionCanvas.h>

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
  // Draw just the empty frame so the histogram and labels clear instead of
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

   {
    const QString minText = QString::number(m_Min, 'g', 4);
    const QString maxText = QString::number(m_Max, 'g', 4);
    const QRect minRect = painter.fontMetrics().boundingRect(minText);
    const QRect maxRect = painter.fontMetrics().boundingRect(maxText);

    int y = contents.height() - minRect.height() + 5;
    painter.setPen(Qt::black);
    painter.drawText(QPoint(11, y + 1), minText);
    painter.setPen(Qt::white);
    painter.drawText(QPoint(10, y), minText);

    y = contents.height() - maxRect.height() + 5;
    const int x = contents.width() - maxRect.width() - 6;
    painter.setPen(Qt::black);
    painter.drawText(QPoint(x, y + 1), maxText);
    painter.setPen(Qt::white);
    painter.drawText(QPoint(x, y), maxText);
  }

  // Opacity curve + control points
  if (m_PiecewiseFunction != nullptr && this->isEnabled() && m_PiecewiseFunction->GetSize() > 0)
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

    // Control-point handles.
    painter.setPen(Qt::black);
    painter.setBrush(QBrush(Qt::white));

    for (int i = 0; i < size; ++i)
    {
      const std::pair<int, int> point = this->FunctionToCanvas(std::make_pair(dp[i * 2], dp[i * 2 + 1]));
      painter.drawEllipse(point.first - 4, point.second - 4, 8, 8);
    }
    painter.setBrush(Qt::NoBrush);
  }
}

void QmitkCombinedTransferFunctionCanvas::mousePressEvent(QMouseEvent * /*mouseEvent*/)
{
  // Display-only: opacity is edited through the view's sliders.
  // Overriding the three mouse handlers as no-ops suppresses the base-class
  // per-point editing that would otherwise take over.
}

void QmitkCombinedTransferFunctionCanvas::mouseMoveEvent(QMouseEvent * /*mouseEvent*/) {}

void QmitkCombinedTransferFunctionCanvas::mouseReleaseEvent(QMouseEvent * /*event*/) {}

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
