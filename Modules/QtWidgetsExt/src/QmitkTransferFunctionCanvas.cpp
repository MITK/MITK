/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkTransferFunctionCanvas.h>

#include <itkObject.h>

#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>

#include <algorithm>

QmitkTransferFunctionCanvas::QmitkTransferFunctionCanvas(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_GrabbedHandle(-1),
    m_Lower(0.0),
    m_Upper(1.0),
    m_Min(0.0),
    m_Max(1.0),
    m_Histogram(nullptr),
    m_ImmediateUpdate(false),
    m_Range(0.0f),
    m_LineEditAvailable(false),
    m_XEdit(nullptr),
    m_YEdit(nullptr)
{
  setEnabled(false);
  setFocusPolicy(Qt::ClickFocus);
}

void QmitkTransferFunctionCanvas::paintEvent(QPaintEvent *ev)
{
  QWidget::paintEvent(ev);
}

std::pair<int, int> QmitkTransferFunctionCanvas::FunctionToCanvas(std::pair<double, double> functionPoint)
{
  return std::make_pair(
    (int)((functionPoint.first - m_Lower) / (m_Upper - m_Lower) * contentsRect().width()) + contentsRect().x(),
    (int)(contentsRect().height() * (1 - functionPoint.second)) + contentsRect().y());
}

std::pair<double, double> QmitkTransferFunctionCanvas::CanvasToFunction(std::pair<int, int> canvasPoint)
{
  return std::make_pair(
    (canvasPoint.first - contentsRect().x()) * (m_Upper - m_Lower) / contentsRect().width() + m_Lower,
    1.0 - (double)(canvasPoint.second - contentsRect().y()) / contentsRect().height());
}

void QmitkTransferFunctionCanvas::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
  int nearHandle = GetNearHandle(mouseEvent->pos().x(), mouseEvent->pos().y());
  if (nearHandle != -1)
  {
    this->DoubleClickOnHandle(nearHandle);
  }
}

/** returns index of a near handle or -1 if none is near
 */
int QmitkTransferFunctionCanvas::GetNearHandle(int, int, unsigned int)
{
  return -1;
}

void QmitkTransferFunctionCanvas::mousePressEvent(QMouseEvent *mouseEvent)
{
  if (m_LineEditAvailable)
  {
    m_XEdit->clear();
    if (m_YEdit)
      m_YEdit->clear();
  }

  const auto pos = mouseEvent->position().toPoint();
  m_GrabbedHandle = GetNearHandle(pos.x(), pos.y());

  if ((mouseEvent->button() & Qt::LeftButton) && m_GrabbedHandle == -1)
  {
    auto [x, value] = this->CanvasToFunction(std::make_pair(pos.x(), pos.y()));
    this->AddFunctionPoint(x, value);
    m_GrabbedHandle = GetNearHandle(pos.x(), pos.y());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else if ((mouseEvent->button() & Qt::RightButton) && m_GrabbedHandle != -1 && this->GetFunctionSize() > 1)
  {
    this->RemoveFunctionPoint(this->GetFunctionX(m_GrabbedHandle));
    m_GrabbedHandle = -1;
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  update();
}

double QmitkTransferFunctionCanvas::ClampGrabbedHandleX(double x)
{
  const int index = m_GrabbedHandle;
  const double current = this->GetFunctionX(index);
  const double pixel = (m_Upper - m_Lower) / this->contentsRect().width();

  // Bounded by where the handle already is, so that one already closer than a
  // pixel to a neighbour stays put rather than being pushed away from it.
  if (index > 0)
    x = std::max(x, std::min(this->GetFunctionX(index - 1) + pixel, current));

  if (index < this->GetFunctionSize() - 1)
    x = std::min(x, std::max(this->GetFunctionX(index + 1) - pixel, current));

  return std::clamp(x, m_Min, m_Max);
}

void QmitkTransferFunctionCanvas::mouseMoveEvent(QMouseEvent *mouseEvent)
{
  if (m_GrabbedHandle != -1)
  {
    const auto pos = mouseEvent->position().toPoint();
    std::pair<double, double> newPos = this->CanvasToFunction(std::make_pair(pos.x(), pos.y()));

    newPos.first = this->ClampGrabbedHandleX(newPos.first);

    // Y Clamping
    {
      if (newPos.second < 0.0)
        newPos.second = 0.0;
      else if (newPos.second > 1.0)
        newPos.second = 1.0;
    }

    // Move selected point
    this->MoveFunctionPoint(m_GrabbedHandle, newPos);

    update();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkTransferFunctionCanvas::mouseReleaseEvent(QMouseEvent *)
{
  update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTransferFunctionCanvas::PaintHistogram(QPainter &p)
{
  if (m_Histogram)
  {
    p.save();

    p.setPen(Qt::gray);

    // The plot is the contents rect, which is where the curve and the coordinate
    // transforms put it; only a margin no wider than the frame keeps that within
    // a pixel of the widget's own corner.
    const QRect contents = this->contentsRect();
    p.translate(contents.topLeft());

    int displayWidth = contents.width();
    int displayHeight = contents.height();

    double windowLeft = m_Lower;
    double windowRight = m_Upper;

    double step = (windowRight - windowLeft) / double(displayWidth);

    double pos = windowLeft;

    for (int x = 0; x < displayWidth; x++)
    {
      double left = pos;
      double right = pos + step;

      float height = m_Histogram->GetRelativeBin(left, right);

      if (height >= 0)
        p.drawLine(x, displayHeight * (1 - height), x, displayHeight);

      pos += step;
    }

    p.restore();
  }
}

void QmitkTransferFunctionCanvas::keyPressEvent(QKeyEvent *e)
{
  if (m_GrabbedHandle == -1)
    return;

  switch (e->key())
  {
    case Qt::Key_Delete:
      if (this->GetFunctionSize() > 1)
      {
        this->RemoveFunctionPoint(GetFunctionX(m_GrabbedHandle));
        m_GrabbedHandle = -1;
      }
      break;

    case Qt::Key_Left:
    case Qt::Key_Right:
    {
      // A pixel rather than a fixed amount of intensity, which on CT would not
      // move the handle visibly and on data normalised to [0, 1] would carry it
      // across the whole axis.
      const double step = (m_Upper - m_Lower) / this->contentsRect().width();
      const double x = this->GetFunctionX(m_GrabbedHandle) + (e->key() == Qt::Key_Left ? -step : step);

      this->MoveFunctionPoint(
        m_GrabbedHandle, std::make_pair(this->ClampGrabbedHandleX(x), this->GetFunctionY(m_GrabbedHandle)));
      break;
    }

    case Qt::Key_Up:
    case Qt::Key_Down:
    {
      const double y = this->GetFunctionY(m_GrabbedHandle) + (e->key() == Qt::Key_Up ? 0.001 : -0.001);

      this->MoveFunctionPoint(
        m_GrabbedHandle, std::make_pair(this->GetFunctionX(m_GrabbedHandle), std::clamp(y, 0.0, 1.0)));
      break;
    }
  }

  update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

// Update immediately while changing the transfer function
void QmitkTransferFunctionCanvas::SetImmediateUpdate(bool state)
{
  m_ImmediateUpdate = state;
}
