/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCategoryToolBar.h"

#include <QmitkIconTheme.h>

#include <QActionEvent>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QStyleOptionToolBar>
#include <QTimer>
#include <QToolButton>

#include <algorithm>
#include <vector>

namespace
{
  constexpr double CAPTION_FONT_SCALE = 0.85;

  constexpr double CAPTION_OPACITY = 0.7;

  // Moving the mouse from one tool bar to an adjacent one leaves the first
  // before it enters the second, which must not make the labels flicker.
  constexpr int HOVER_LABEL_GRACE_PERIOD_IN_MS = 100;

  constexpr int HOVER_LABEL_SPACING = 2;

  QString HoverLabelStyleSheet()
  {
    const auto darkTheme = QmitkIconTheme::IsDarkTheme();

    const auto surface = darkTheme ? QStringLiteral("#3f3f46") : QStringLiteral("palette(base)");
    const auto border = darkTheme ? QStringLiteral("#54545a") : QStringLiteral("palette(mid)");

    return QStringLiteral("QLabel { background-color: %1; border: 1px solid %2; border-radius: 3px; padding: 0px 4px; }")
      .arg(surface, border);
  }
}

QmitkCategoryToolBar::QmitkCategoryToolBar(const QString& category, QWidget* parent)
  : QToolBar(parent),
    m_Category(category),
    m_CategoryLabel(CategoryLabel::Hidden),
    m_ExtensionButton(this->findChild<QToolButton*>(QStringLiteral("qt_toolbar_ext_button"), Qt::FindDirectChildrenOnly))
{
  if (nullptr != m_ExtensionButton)
    m_ExtensionButton->installEventFilter(this);

  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, qOverload<>(&QWidget::update));
}

QmitkCategoryToolBar::~QmitkCategoryToolBar()
{
  // The hover label is a child of the window, which may outlive this tool bar.
  delete m_HoverLabel;
}

QString QmitkCategoryToolBar::GetCategory() const
{
  return m_Category;
}

QmitkCategoryToolBar::CategoryLabel QmitkCategoryToolBar::GetCategoryLabel() const
{
  return m_CategoryLabel;
}

void QmitkCategoryToolBar::SetCategoryLabel(CategoryLabel categoryLabel)
{
  if (categoryLabel == m_CategoryLabel)
    return;

  m_CategoryLabel = categoryLabel;

  if (CategoryLabel::OnHover != m_CategoryLabel)
    this->HideHoverLabel();

  this->updateGeometry();
  this->update();
}

QSize QmitkCategoryToolBar::sizeHint() const
{
  auto size = QToolBar::sizeHint();

  if (this->ReservesCaptionSpace())
  {
    const QFontMetrics metrics(this->GetCaptionFont());
    const auto captionWidth = this->GetButtonsLeft() + metrics.horizontalAdvance(m_Category) + this->layout()->contentsMargins().right();

    size.rheight() += metrics.height();
    size.setWidth(std::max(size.width(), captionWidth));
  }

  return size;
}

QSize QmitkCategoryToolBar::minimumSizeHint() const
{
  auto size = QToolBar::minimumSizeHint();

  // A tool bar narrower than its caption elides the caption instead.
  if (this->ReservesCaptionSpace())
    size.rheight() += QFontMetrics(this->GetCaptionFont()).height();

  return size;
}

void QmitkCategoryToolBar::actionEvent(QActionEvent* event)
{
  QToolBar::actionEvent(event);

  if (QEvent::ActionAdded == event->type())
  {
    if (auto* widget = this->widgetForAction(event->action()); nullptr != widget)
      this->layout()->setAlignment(widget, Qt::AlignBottom);
  }
}

void QmitkCategoryToolBar::changeEvent(QEvent* event)
{
  QToolBar::changeEvent(event);

  if (QEvent::FontChange == event->type() || QEvent::StyleChange == event->type())
    this->updateGeometry();
}

void QmitkCategoryToolBar::enterEvent(QEnterEvent* event)
{
  QToolBar::enterEvent(event);

  if (CategoryLabel::OnHover != m_CategoryLabel || m_Category.isEmpty())
    return;

  this->ShowHoverLabels();
}

void QmitkCategoryToolBar::leaveEvent(QEvent* event)
{
  QToolBar::leaveEvent(event);

  if (CategoryLabel::OnHover != m_CategoryLabel || m_Category.isEmpty())
    return;

  QTimer::singleShot(HOVER_LABEL_GRACE_PERIOD_IN_MS, this, &QmitkCategoryToolBar::HideHoverLabelsUnlessHovered);
}

void QmitkCategoryToolBar::hideEvent(QHideEvent* event)
{
  QToolBar::hideEvent(event);
  this->HideHoverLabel();
}

void QmitkCategoryToolBar::moveEvent(QMoveEvent* event)
{
  QToolBar::moveEvent(event);
  this->UpdateShownHoverLabels();
}

void QmitkCategoryToolBar::resizeEvent(QResizeEvent* event)
{
  QToolBar::resizeEvent(event);
  this->UpdateShownHoverLabels();
}

void QmitkCategoryToolBar::paintEvent(QPaintEvent* event)
{
  QToolBar::paintEvent(event);

  // The rows of an expanded tool bar start right at the top.
  if (!this->ReservesCaptionSpace() || this->IsExpanded())
    return;

  const auto captionRect = this->GetCaptionRect();

  QColor color(QmitkIconTheme::GetColor());
  color.setAlphaF(CAPTION_OPACITY);

  QPainter painter(this);
  painter.setFont(this->GetCaptionFont());
  painter.setPen(color);
  painter.drawText(captionRect, Qt::AlignCenter,
    painter.fontMetrics().elidedText(m_Category, Qt::ElideRight, captionRect.width()));
}

void QmitkCategoryToolBar::mousePressEvent(QMouseEvent* event)
{
  // Clicking the caption of a tool bar with overflowing buttons shows them.
  if (Qt::LeftButton == event->button() && this->ReservesCaptionSpace() && !this->IsExpanded() &&
      nullptr != m_ExtensionButton && m_ExtensionButton->isVisible() &&
      this->GetCaptionRect().contains(event->position().toPoint()))
  {
    m_ExtensionButton->click();
    return;
  }

  QToolBar::mousePressEvent(event);
}

bool QmitkCategoryToolBar::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == m_ExtensionButton && (QEvent::Move == event->type() || QEvent::Resize == event->type()))
    this->AlignExtensionButton();

  return QToolBar::eventFilter(watched, event);
}

bool QmitkCategoryToolBar::ReservesCaptionSpace() const
{
  return CategoryLabel::AboveButtons == m_CategoryLabel && !m_Category.isEmpty() && Qt::Horizontal == this->orientation();
}

bool QmitkCategoryToolBar::IsExpanded() const
{
  // The tool bar layout checks the extension button while the overflowing
  // buttons are shown in additional rows.
  return nullptr != m_ExtensionButton && m_ExtensionButton->isChecked();
}

QFont QmitkCategoryToolBar::GetCaptionFont() const
{
  auto font = this->font();

  if (font.pointSizeF() > 0.0)
  {
    font.setPointSizeF(font.pointSizeF() * CAPTION_FONT_SCALE);
  }
  else
  {
    font.setPixelSize(std::max(1, qRound(font.pixelSize() * CAPTION_FONT_SCALE)));
  }

  return font;
}

int QmitkCategoryToolBar::GetButtonsLeft() const
{
  QStyleOptionToolBar option;
  this->initStyleOption(&option);

  const auto handleExtent = this->isMovable()
    ? this->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, &option, this)
    : 0;

  return this->layout()->contentsMargins().left() + handleExtent;
}

QRect QmitkCategoryToolBar::GetCaptionRect() const
{
  const auto margins = this->layout()->contentsMargins();
  const QFontMetrics metrics(this->GetCaptionFont());
  const auto left = this->GetButtonsLeft();

  // The last tool bar in a row is stretched to the end of the row, so the
  // caption is centered above the buttons instead of the whole tool bar.
  const auto right = std::min(this->width() - margins.right(), std::max(this->GetButtonsRight(), left + metrics.horizontalAdvance(m_Category)));

  return QRect(left, margins.top(), right - left, metrics.height());
}

int QmitkCategoryToolBar::GetButtonsRight() const
{
  // Hidden children, like buttons moved into the overflow, do not count.
  const auto children = this->childrenRect();

  return children.isValid()
    ? children.left() + children.width()
    : this->width() - this->layout()->contentsMargins().right();
}

void QmitkCategoryToolBar::AlignExtensionButton()
{
  // The tool bar layout places the extension button at the top, apart from
  // the bottom-aligned buttons it belongs to.
  if (Qt::Horizontal != this->orientation() || this->IsExpanded())
    return;

  const auto y = this->height() - this->layout()->contentsMargins().bottom() - m_ExtensionButton->height();

  if (y != m_ExtensionButton->y())
    m_ExtensionButton->move(m_ExtensionButton->x(), y);
}

QList<QmitkCategoryToolBar*> QmitkCategoryToolBar::GetToolBarsOfSameWindow() const
{
  auto* window = this->parentWidget();

  return nullptr != window
    ? window->findChildren<QmitkCategoryToolBar*>(Qt::FindDirectChildrenOnly)
    : QList<QmitkCategoryToolBar*>();
}

void QmitkCategoryToolBar::ShowHoverLabels()
{
  auto* window = this->parentWidget();

  if (nullptr == window)
    return;

  struct Placement
  {
    QmitkCategoryToolBar* ToolBar;
    QRect Bounds;
    int NaturalWidth;
    int Width;
  };

  std::vector<Placement> placements;
  const auto styleSheet = HoverLabelStyleSheet();

  for (auto* toolBar : this->GetToolBarsOfSameWindow())
  {
    if (toolBar->m_Category.isEmpty() || !toolBar->isVisible() || toolBar->isFloating() || Qt::Horizontal != toolBar->orientation())
    {
      toolBar->HideHoverLabel();
      continue;
    }

    auto& label = toolBar->m_HoverLabel;

    if (label.isNull())
    {
      label = new QLabel(window);
      label->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    // Labels are laid out again whenever a tool bar moves, e.g. many times
    // while one is dragged, but restyling them is expensive.
    if (label->styleSheet() != styleSheet)
      label->setStyleSheet(styleSheet);

    label->setFont(toolBar->GetCaptionFont());
    label->setText(toolBar->m_Category);
    label->adjustSize();

    // The last tool bar in a row is stretched to the end of the row, so the
    // label is centered above the part of the tool bar that has content.
    const auto contentWidth = std::min(toolBar->width(), toolBar->GetButtonsRight() + toolBar->layout()->contentsMargins().right());
    const QRect bounds(toolBar->mapTo(window, QPoint(0, 0)), QSize(contentWidth, toolBar->height()));

    placements.push_back({ toolBar, bounds, label->width(), std::min(label->width(), bounds.width() - HOVER_LABEL_SPACING) });
  }

  std::sort(placements.begin(), placements.end(), [](const Placement& lhs, const Placement& rhs) {
    return lhs.Bounds.top() != rhs.Bounds.top()
      ? lhs.Bounds.top() < rhs.Bounds.top()
      : lhs.Bounds.left() < rhs.Bounds.left();
  });

  const auto labelLeft = [](const Placement& placement) {
    return placement.Bounds.left() + (placement.Bounds.width() - placement.Width) / 2;
  };

  // Each label is centered above its tool bar. A label wider than its tool
  // bar extends by the same amount on both sides into room not taken by the
  // labels next to it in the same row, and is truncated beyond that.
  for (std::size_t i = 0; i < placements.size(); ++i)
  {
    auto& placement = placements[i];

    if (placement.Width >= placement.NaturalWidth)
      continue;

    const auto* previous = i > 0 && placements[i - 1].Bounds.top() == placement.Bounds.top()
      ? &placements[i - 1]
      : nullptr;

    const auto* next = i + 1 < placements.size() && placements[i + 1].Bounds.top() == placement.Bounds.top()
      ? &placements[i + 1]
      : nullptr;

    const auto leftLimit = nullptr != previous
      ? labelLeft(*previous) + previous->Width + HOVER_LABEL_SPACING
      : 0;

    const auto rightLimit = nullptr != next
      ? labelLeft(*next) - HOVER_LABEL_SPACING
      : window->width();

    const auto room = std::min(placement.Bounds.left() - leftLimit, rightLimit - (placement.Bounds.left() + placement.Bounds.width()));

    if (room > 0)
      placement.Width = std::min(placement.NaturalWidth, placement.Bounds.width() + 2 * room);
  }

  for (const auto& placement : placements)
  {
    auto* label = placement.ToolBar->m_HoverLabel.data();

    if (placement.Width < placement.NaturalWidth)
    {
      const auto metrics = label->fontMetrics();
      const auto chromeWidth = placement.NaturalWidth - metrics.horizontalAdvance(label->text());

      label->setText(metrics.elidedText(label->text(), Qt::ElideRight, placement.Width - chromeWidth));
      label->adjustSize();
    }

    auto y = placement.Bounds.top() - label->height();

    // A window without menu bar may have no room above its top tool bars.
    if (y < 0)
      y = placement.Bounds.top() + placement.Bounds.height();

    label->move(placement.Bounds.left() + (placement.Bounds.width() - label->width()) / 2, y);
    label->show();
    label->raise();
  }
}

void QmitkCategoryToolBar::UpdateShownHoverLabels()
{
  // Shown labels would stay behind when tool bars move, e.g. when one is
  // dragged into another row.
  const auto toolBars = this->GetToolBarsOfSameWindow();

  const auto areShown = std::any_of(toolBars.cbegin(), toolBars.cend(), [](const QmitkCategoryToolBar* toolBar) {
    return !toolBar->m_HoverLabel.isNull() && toolBar->m_HoverLabel->isVisible();
  });

  if (areShown)
    this->ShowHoverLabels();
}

void QmitkCategoryToolBar::HideHoverLabel()
{
  if (!m_HoverLabel.isNull())
    m_HoverLabel->hide();
}

void QmitkCategoryToolBar::HideHoverLabelsUnlessHovered()
{
  const auto toolBars = this->GetToolBarsOfSameWindow();

  const auto isHovered = std::any_of(toolBars.cbegin(), toolBars.cend(), [](const QmitkCategoryToolBar* toolBar) {
    return !toolBar->GetCategory().isEmpty() && toolBar->underMouse();
  });

  if (isHovered)
    return;

  for (auto* toolBar : toolBars)
    toolBar->HideHoverLabel();
}
