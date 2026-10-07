/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkSpeechBubbleFrame.h>

#include <QmitkIconTheme.h>

#include <QApplication>
#include <QChildEvent>
#include <QPainter>
#include <QPainterPath>

#include <cmath>

namespace
{
  constexpr qreal RADIUS = 8;
  constexpr qreal TAIL_WIDTH = 18;
  constexpr qreal TAIL_HEIGHT = 14;

  // Between the outline and the content.
  constexpr int PADDING = 4;

  constexpr qreal BORDER_WIDTH = 1;
  constexpr qreal FOCUS_BORDER_WIDTH = 1.5;

  // The blue of the VoxTell icon.
  const QColor FOCUS_COLOR("#00adff");

  // Like the icon: a vertical right side and a slanted left side.
  QPainterPath GetBubblePath(const QRectF& body)
  {
    const auto tailRight = body.right() - 3 * RADIUS;
    const auto tailLeft = tailRight - TAIL_WIDTH;
    const auto diameter = 2 * RADIUS;

    QPainterPath path;
    path.moveTo(body.left() + RADIUS, body.top());
    path.arcTo(QRectF(body.right() - diameter, body.top(), diameter, diameter), 90, -90);
    path.arcTo(QRectF(body.right() - diameter, body.bottom() - diameter, diameter, diameter), 0, -90);
    path.lineTo(tailRight, body.bottom());
    path.lineTo(tailRight, body.bottom() + TAIL_HEIGHT);
    path.lineTo(tailLeft, body.bottom());
    path.arcTo(QRectF(body.left(), body.bottom() - diameter, diameter, diameter), 270, -90);
    path.arcTo(QRectF(body.left(), body.top(), diameter, diameter), 180, -90);
    path.closeSubpath();

    return path;
  }
}

QmitkSpeechBubbleFrame::QmitkSpeechBubbleFrame(QWidget* parent)
  : QWidget(parent)
{
  const int margin = PADDING + static_cast<int>(std::ceil(FOCUS_BORDER_WIDTH));
  this->setContentsMargins(margin, margin, margin, margin + static_cast<int>(TAIL_HEIGHT));

  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, qOverload<>(&QWidget::update));
}

QmitkSpeechBubbleFrame::~QmitkSpeechBubbleFrame() = default;

void QmitkSpeechBubbleFrame::childEvent(QChildEvent* event)
{
  if (event->type() == QEvent::ChildAdded)
    event->child()->installEventFilter(this);

  QWidget::childEvent(event);
}

bool QmitkSpeechBubbleFrame::eventFilter(QObject* watched, QEvent* event)
{
  switch (event->type())
  {
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::EnabledChange:
      this->update();
      break;

    default:
      break;
  }

  return QWidget::eventFilter(watched, event);
}

void QmitkSpeechBubbleFrame::paintEvent(QPaintEvent* /*event*/)
{
  const auto* content = this->findChild<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
  const bool enabled = content == nullptr || content->isEnabled();

  const auto* focusWidget = QApplication::focusWidget();
  const bool focused = enabled && focusWidget != nullptr && this->isAncestorOf(focusWidget);

  QColor fill;
  QColor border;

  // The colors of the text fields of the theme. The dark theme sets them in its
  // style sheet and leaves the palette untouched.
  if (QmitkIconTheme::IsDarkTheme())
  {
    fill = enabled ? QColor("#333337") : QColor("#2d2d30");
    border = QColor("#434346");
  }
  else
  {
    fill = this->palette().color(enabled ? QPalette::Base : QPalette::Window);
    border = this->palette().color(QPalette::Mid);

    if (!enabled)
      border = border.lighter(130);
  }

  const auto borderWidth = focused ? FOCUS_BORDER_WIDTH : BORDER_WIDTH;
  const auto inset = borderWidth / 2;
  const auto body = QRectF(this->rect()).adjusted(inset, inset, -inset, -inset - TAIL_HEIGHT);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(QPen(focused ? FOCUS_COLOR : border, borderWidth));
  painter.setBrush(fill);
  painter.drawPath(GetBubblePath(body));
}
