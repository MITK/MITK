/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFloatingTextPopup.h"

#include <QPainter>
#include <QTextDocument>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QCursor>
#include <algorithm>

QmitkFloatingTextPopup::QmitkFloatingTextPopup(QWidget* parent)
  : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint |
            Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus)
{
  this->setAttribute(Qt::WA_TranslucentBackground);
  this->setAttribute(Qt::WA_ShowWithoutActivating);
  this->setAttribute(Qt::WA_TransparentForMouseEvents); // Don't interfere with mouse events
  this->setFocusPolicy(Qt::NoFocus);
}

QmitkFloatingTextPopup::~QmitkFloatingTextPopup()
{
  this->removeGlobalEventFilter();
}

QString QmitkFloatingTextPopup::GetHtmlContent() const
{
  return m_HtmlContent;
}

void QmitkFloatingTextPopup::SetHtmlContent(const QString& html)
{
  if (m_HtmlContent != html)
  {
    m_HtmlContent = html;
    this->updateGeometry();
    this->updateVisibility();
    this->update();
  }
}

int QmitkFloatingTextPopup::GetOpacity() const
{
  return m_Opacity;
}

void QmitkFloatingTextPopup::SetOpacity(int opacity)
{
  m_Opacity = std::clamp(opacity, 0, 255);
  this->update();
}

bool QmitkFloatingTextPopup::IsEnabled() const
{
  return m_Enabled;
}

void QmitkFloatingTextPopup::SetEnabled(bool enabled)
{
  if (m_Enabled != enabled)
  {
    m_Enabled = enabled;

    if (m_Enabled)
    {
      this->installGlobalEventFilter();
      this->updateVisibility();
    }
    else
    {
      this->removeGlobalEventFilter();
      this->hide();
    }
  }
}

void QmitkFloatingTextPopup::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Draw semi-transparent background with rounded corners
  QRectF backgroundRect(0, 0, width(), height());
  painter.setBrush(QColor(0, 0, 0, m_Opacity));
  painter.setPen(QColor(80, 80, 80, m_Opacity));
  painter.drawRoundedRect(backgroundRect, 4, 4);

  // Render HTML content
  if (!m_HtmlContent.isEmpty())
  {
    QTextDocument textDoc;
    textDoc.setDefaultStyleSheet(qApp->styleSheet());
    textDoc.setHtml(m_HtmlContent);
    textDoc.setTextWidth(width() - 2 * m_Padding);

    painter.translate(m_Padding, 0);
    textDoc.drawContents(&painter);
  }
}

QSize QmitkFloatingTextPopup::sizeHint() const
{
  if (m_HtmlContent.isEmpty())
    return QSize(100, 50);

  QTextDocument textDoc;
  textDoc.setDefaultStyleSheet(qApp->styleSheet());
  textDoc.setHtml(m_HtmlContent);
  textDoc.setTextWidth(m_MaxWidth - 2 * m_Padding);

  QSizeF docSize = textDoc.size();
  int width = std::min(m_MaxWidth, static_cast<int>(docSize.width()) + 2 * m_Padding);
  int height = static_cast<int>(docSize.height()) + 2 * m_Padding;

  return QSize(width, height);
}

bool QmitkFloatingTextPopup::eventFilter(QObject* watched, QEvent* event)
{
  // We're filtering application-wide events
  if (event->type() == QEvent::MouseMove)
  {
    auto* mouseEvent = static_cast<QMouseEvent*>(event);
    this->updatePosition(mouseEvent->globalPosition().toPoint());
    return false; // Don't consume the event
  }

  return QWidget::eventFilter(watched, event);
}

void QmitkFloatingTextPopup::updatePosition(const QPoint& globalPos)
{
  if (!m_Enabled || m_HtmlContent.isEmpty())
    return;

  QPoint popupPos = globalPos + m_CursorOffset;
  popupPos = constrainToScreen(popupPos, size());

  move(popupPos);

  if (!isVisible())
    this->show();

  this->raise();
}

void QmitkFloatingTextPopup::updateGeometry()
{
  this->resize(sizeHint());
  this->adjustSize();
}

void QmitkFloatingTextPopup::updateVisibility()
{
  if (m_Enabled && !m_HtmlContent.isEmpty())
  {
    // Update position and show
    this->updatePosition(QCursor::pos());
  }
  else
  {
    hide();
  }
}

QPoint QmitkFloatingTextPopup::constrainToScreen(const QPoint& pos, const QSize& size) const
{
  QScreen* screen = QGuiApplication::screenAt(pos);
  if (!screen)
    screen = QGuiApplication::primaryScreen();

  QRect screenGeometry = screen->availableGeometry();
  QPoint constrainedPos = pos;

  // Ensure popup stays within screen bounds
  if (constrainedPos.x() + size.width() > screenGeometry.right())
    constrainedPos.setX(screenGeometry.right() - size.width());

  if (constrainedPos.y() + size.height() > screenGeometry.bottom())
    constrainedPos.setY(screenGeometry.bottom() - size.height());

  if (constrainedPos.x() < screenGeometry.left())
    constrainedPos.setX(screenGeometry.left());

  if (constrainedPos.y() < screenGeometry.top())
    constrainedPos.setY(screenGeometry.top());

  return constrainedPos;
}

void QmitkFloatingTextPopup::installGlobalEventFilter()
{
  if (!m_EventFilterInstalled)
  {
    qApp->installEventFilter(this);
    m_EventFilterInstalled = true;
  }
}

void QmitkFloatingTextPopup::removeGlobalEventFilter()
{
  if (m_EventFilterInstalled)
  {
    qApp->removeEventFilter(this);
    m_EventFilterInstalled = false;
  }
}