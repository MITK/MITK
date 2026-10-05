/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderWindowMenuBar.h"

#include <QCursor>
#include <QEnterEvent>
#include <QGraphicsOpacityEffect>
#include <QMenu>
#include <QPainter>
#include <QToolButton>
#include <QVariantAnimation>

#include <algorithm>
#include <cmath>

namespace
{
  /* The full appearance at the default scale, in logical pixels. */
  constexpr int ICON_SIZE = 20;
  constexpr int BUTTON_PADDING = 4;
  constexpr int BAR_MARGIN = 2;
  constexpr int BUTTON_SPACING = 2;
  constexpr int DEFAULT_CORNER_RADIUS = 8;

  /* For the whole way between the resting and the full appearance. Turning
   * back midway takes the share of the way already covered. */
  constexpr int ANIMATION_DURATION = 120;

  /* In heights of the full bar. A bar engages when the cursor comes this close
   * to its full outline and releases it only beyond the larger distance, so
   * that a cursor resting at the border does not make it flicker. */
  constexpr double ENGAGE_DISTANCE = 1.0;
  constexpr double RELEASE_DISTANCE = 1.5;

  int Scaled(int value, double scale)
  {
    return static_cast<int>(std::lround(value * scale));
  }
}

QmitkRenderWindowMenuBar::QmitkRenderWindowMenuBar(Corner corner, QWidget* parent)
  : QWidget(parent),
    m_Corner(corner),
    m_Scale(1.0),
    m_RestingScale(1.0),
    m_RestingOpacity(1.0),
    m_CornerRadius(DEFAULT_CORNER_RADIUS),
    m_Emphasis(0.0),
    m_Engaged(false),
    m_MenuOpen(false),
    m_Animation(new QVariantAnimation(this)),
    m_OpacityEffect(new QGraphicsOpacityEffect(this))
{
  this->setGraphicsEffect(m_OpacityEffect);

  // Clicks and wheel ticks on the margins would otherwise reach the render
  // window and act on the scene.
  this->setAttribute(Qt::WA_NoMousePropagation);

  m_Animation->setEasingCurve(QEasingCurve::OutCubic);

  connect(m_Animation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
    this->SetEmphasis(value.toDouble());
  });

  this->hide();
}

QmitkRenderWindowMenuBar::~QmitkRenderWindowMenuBar() = default;

QToolButton* QmitkRenderWindowMenuBar::AddButton(const QIcon& icon)
{
  auto* button = new QToolButton(this);
  button->setIcon(icon);
  button->setAutoRaise(true);
  button->installEventFilter(this);

  m_Buttons.push_back(button);
  this->ApplyAppearance();

  return button;
}

QToolButton* QmitkRenderWindowMenuBar::AddMenuButton(const QIcon& icon, QMenu* menu)
{
  auto* button = this->AddButton(icon);

  // Opened by hand rather than set on the button, which would draw a menu
  // indicator into the icon.
  connect(button, &QToolButton::clicked, menu, [button, menu]() {
    menu->exec(button->mapToGlobal(QPoint(0, button->height())));
  });

  this->KeepFullWhileOpen(menu);

  return button;
}

void QmitkRenderWindowMenuBar::KeepFullWhileOpen(QMenu* menu)
{
  connect(menu, &QMenu::aboutToShow, this, [this]() {
    m_MenuOpen = true;
    this->SetEngaged(true);
  });

  connect(menu, &QMenu::aboutToHide, this, [this]() {
    m_MenuOpen = false;
    this->UpdateProximity(this->parentWidget()->mapFromGlobal(QCursor::pos()));
  });
}

void QmitkRenderWindowMenuBar::SetScale(double scale)
{
  m_Scale = scale;
  this->ApplyAppearance();
}

void QmitkRenderWindowMenuBar::SetRestingAppearance(double scale, double opacity)
{
  m_RestingScale = scale;
  m_RestingOpacity = opacity;
  this->ApplyAppearance();
}

bool QmitkRenderWindowMenuBar::HasShownButtons() const
{
  return std::any_of(m_Buttons.begin(), m_Buttons.end(), [](const QToolButton* button) {
    return !button->isHidden();
  });
}

QSize QmitkRenderWindowMenuBar::GetFullSize() const
{
  return this->ComputeSize(m_Scale);
}

void QmitkRenderWindowMenuBar::Reveal()
{
  if (this->isHidden())
  {
    m_Animation->stop();
    m_Engaged = false;
    this->SetEmphasis(0.0);
    this->show();
  }

  this->raise();
}

void QmitkRenderWindowMenuBar::Conceal()
{
  m_Animation->stop();
  m_Engaged = false;
  this->hide();
}

void QmitkRenderWindowMenuBar::Dock()
{
  const int x = Corner::TopRight == m_Corner
    ? this->parentWidget()->width() - this->width()
    : 0;

  this->move(x, 0);
}

void QmitkRenderWindowMenuBar::UpdateProximity(const QPoint& cursor)
{
  if (m_MenuOpen || this->isHidden())
    return;

  // Measured against the full outline rather than the current one, which
  // would otherwise grow towards the cursor and pull the border along.
  const auto fullSize = this->GetFullSize();

  const int x = Corner::TopRight == m_Corner
    ? this->parentWidget()->width() - fullSize.width()
    : 0;

  const int reach = static_cast<int>(std::lround(fullSize.height() * (m_Engaged ? RELEASE_DISTANCE : ENGAGE_DISTANCE)));
  const auto zone = QRect(QPoint(x, 0), fullSize).adjusted(-reach, -reach, reach, reach);

  this->SetEngaged(zone.contains(cursor));
}

int QmitkRenderWindowMenuBar::GetCornerRadius() const
{
  return m_CornerRadius;
}

void QmitkRenderWindowMenuBar::SetCornerRadius(int radius)
{
  m_CornerRadius = radius;
  this->update();
}

bool QmitkRenderWindowMenuBar::eventFilter(QObject* watched, QEvent* event)
{
  if (QEvent::ShowToParent == event->type() || QEvent::HideToParent == event->type())
    this->ApplyAppearance();

  return QWidget::eventFilter(watched, event);
}

void QmitkRenderWindowMenuBar::enterEvent(QEnterEvent* event)
{
  // The parent does not see the mouse moves over the bar.
  this->SetEngaged(true);
  QWidget::enterEvent(event);
}

void QmitkRenderWindowMenuBar::paintEvent(QPaintEvent* /*event*/)
{
  const double radius = std::min({ m_CornerRadius * this->GetCurrentScale(),
                                   static_cast<double>(this->width()),
                                   static_cast<double>(this->height()) });

  // Reaching beyond the two edges the bar is flush with leaves only the
  // corner facing the image rounded.
  const auto outline = Corner::TopRight == m_Corner
    ? QRectF(0.0, -radius, this->width() + radius, this->height() + radius)
    : QRectF(-radius, -radius, this->width() + radius, this->height() + radius);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::NoPen);
  painter.setBrush(this->palette().color(this->backgroundRole()));
  painter.drawRoundedRect(outline, radius, radius);
}

bool QmitkRenderWindowMenuBar::ChangesAppearanceAtRest() const
{
  return m_RestingScale < 1.0 || m_RestingOpacity < 1.0;
}

double QmitkRenderWindowMenuBar::GetCurrentScale() const
{
  return m_Scale * (m_RestingScale + (1.0 - m_RestingScale) * m_Emphasis);
}

QSize QmitkRenderWindowMenuBar::ComputeSize(double scale) const
{
  const auto shownButtons = static_cast<int>(std::count_if(m_Buttons.begin(), m_Buttons.end(), [](const QToolButton* button) {
    return !button->isHidden();
  }));

  const int buttonSize = Scaled(ICON_SIZE, scale) + 2 * Scaled(BUTTON_PADDING, scale);
  const int margin = Scaled(BAR_MARGIN, scale);
  const int spacing = Scaled(BUTTON_SPACING, scale);

  const int width = 2 * margin + shownButtons * buttonSize + std::max(0, shownButtons - 1) * spacing;

  return QSize(width, 2 * margin + buttonSize);
}

void QmitkRenderWindowMenuBar::SetEngaged(bool engaged)
{
  if (engaged == m_Engaged)
    return;

  m_Engaged = engaged;
  m_Animation->stop();

  const double target = engaged ? 1.0 : 0.0;

  if (!this->ChangesAppearanceAtRest() || this->isHidden())
  {
    this->SetEmphasis(target);
    return;
  }

  const auto duration = std::lround(ANIMATION_DURATION * std::abs(target - m_Emphasis));

  m_Animation->setStartValue(m_Emphasis);
  m_Animation->setEndValue(target);
  m_Animation->setDuration(std::max(1, static_cast<int>(duration)));
  m_Animation->start();
}

void QmitkRenderWindowMenuBar::SetEmphasis(double emphasis)
{
  m_Emphasis = emphasis;
  this->ApplyAppearance();
}

void QmitkRenderWindowMenuBar::ApplyAppearance()
{
  const double scale = this->GetCurrentScale();

  const int iconSize = Scaled(ICON_SIZE, scale);
  const int buttonSize = iconSize + 2 * Scaled(BUTTON_PADDING, scale);
  const int margin = Scaled(BAR_MARGIN, scale);
  const int spacing = Scaled(BUTTON_SPACING, scale);

  int x = margin;

  for (auto* button : m_Buttons)
  {
    if (button->isHidden())
      continue;

    button->setIconSize(QSize(iconSize, iconSize));
    button->setGeometry(x, margin, buttonSize, buttonSize);
    x += buttonSize + spacing;
  }

  this->resize(this->ComputeSize(scale));
  this->Dock();

  m_OpacityEffect->setOpacity(m_RestingOpacity + (1.0 - m_RestingOpacity) * m_Emphasis);
  this->update();
}
