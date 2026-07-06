/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNLinkSeamWidget.h"

#include <QmitkMxNMultiWidget.h>

#include <mitkExceptionMacro.h>
#include <mitkLog.h>

#include <QEvent>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QSplitter>
#include <QToolTip>

#include <algorithm>
#include <array>

namespace
{
  constexpr int SeamSegmentWidth = 3;
  constexpr int ChipSize = 20;
  constexpr int ChipSpacing = 2;
  constexpr int PillPadding = 4;
  constexpr int SeamActivationDistance = 12;

  const QColor PillScrim(0, 0, 0, 140);          // matches the overlay scrim tone
  const QColor NeutralChip(255, 255, 255, 140);  // 55 % white
  const QColor ActiveGlyph(255, 255, 255, 216);

  constexpr std::array<QmitkMxNSyncDimension, 4> NavigationBundle{
    QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
    QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair
  };

  const char* DimensionName(QmitkMxNSyncDimension dimension)
  {
    switch (dimension)
    {
      case QmitkMxNSyncDimension::Pan:         return "pan";
      case QmitkMxNSyncDimension::Zoom:        return "zoom";
      case QmitkMxNSyncDimension::Slice:       return "slice";
      case QmitkMxNSyncDimension::Crosshair:   return "crosshair";
      case QmitkMxNSyncDimension::Orientation: return "orientation";
      case QmitkMxNSyncDimension::Windowing:   return "windowing";
      case QmitkMxNSyncDimension::Lut:         return "LUT";
    }
    return "";
  }

  /** Compact dimension glyphs, drawn rather than shipped as icons so they
   *  inherit the chip's foreground color. */
  void DrawDimensionGlyph(QPainter& painter, const QRect& rect, QmitkMxNSyncDimension dimension,
                          const QColor& color)
  {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(color, 1.4));
    painter.setBrush(Qt::NoBrush);

    const QPointF center = QRectF(rect).center();
    const qreal r = rect.width() * 0.28;

    switch (dimension)
    {
      case QmitkMxNSyncDimension::Pan:  // four-way arrows
      {
        painter.drawLine(QPointF(center.x() - r, center.y()), QPointF(center.x() + r, center.y()));
        painter.drawLine(QPointF(center.x(), center.y() - r), QPointF(center.x(), center.y() + r));
        for (const auto& tip : { QPointF(center.x() - r, center.y()), QPointF(center.x() + r, center.y()),
                                 QPointF(center.x(), center.y() - r), QPointF(center.x(), center.y() + r) })
        {
          const QPointF toCenter = (center - tip) * 0.35;
          const QPointF normal(-toCenter.y(), toCenter.x());
          painter.drawLine(tip, tip + toCenter + normal * 0.8);
          painter.drawLine(tip, tip + toCenter - normal * 0.8);
        }
        break;
      }
      case QmitkMxNSyncDimension::Zoom:  // magnifier
        painter.drawEllipse(QPointF(center.x() - r * 0.3, center.y() - r * 0.3), r * 0.8, r * 0.8);
        painter.drawLine(QPointF(center.x() + r * 0.3, center.y() + r * 0.3),
                         QPointF(center.x() + r, center.y() + r));
        break;
      case QmitkMxNSyncDimension::Slice:  // card stack
        for (int i = -1; i <= 1; ++i)
        {
          painter.drawLine(QPointF(center.x() - r, center.y() + i * r * 0.7),
                           QPointF(center.x() + r, center.y() + i * r * 0.7));
        }
        break;
      case QmitkMxNSyncDimension::Crosshair:  // plus with center gap
        painter.drawLine(QPointF(center.x() - r, center.y()), QPointF(center.x() - r * 0.3, center.y()));
        painter.drawLine(QPointF(center.x() + r * 0.3, center.y()), QPointF(center.x() + r, center.y()));
        painter.drawLine(QPointF(center.x(), center.y() - r), QPointF(center.x(), center.y() - r * 0.3));
        painter.drawLine(QPointF(center.x(), center.y() + r * 0.3), QPointF(center.x(), center.y() + r));
        break;
      case QmitkMxNSyncDimension::Orientation:  // axes tripod
        painter.drawLine(center, QPointF(center.x(), center.y() - r));
        painter.drawLine(center, QPointF(center.x() + r, center.y() + r * 0.5));
        painter.drawLine(center, QPointF(center.x() - r, center.y() + r * 0.5));
        break;
      case QmitkMxNSyncDimension::Windowing:  // half-filled contrast disc
        painter.drawEllipse(center, r, r);
        painter.setBrush(color);
        painter.drawPie(QRectF(center.x() - r, center.y() - r, 2 * r, 2 * r), 90 * 16, 180 * 16);
        break;
      case QmitkMxNSyncDimension::Lut:  // gradient bar
      {
        QLinearGradient gradient(QPointF(center.x() - r, center.y()), QPointF(center.x() + r, center.y()));
        gradient.setColorAt(0.0, QColor(color.red(), color.green(), color.blue(), 40));
        gradient.setColorAt(1.0, color);
        painter.fillRect(QRectF(center.x() - r, center.y() - r * 0.5, 2 * r, r), gradient);
        break;
      }
    }
    painter.restore();
  }
}

QmitkMxNLinkSeamWidget::QmitkMxNLinkSeamWidget(QmitkMxNMultiWidget* editor,
                                               QSplitterHandle* handle,
                                               const QString& firstId,
                                               const QString& secondId,
                                               QmitkRenderWindowProximity* proximity)
  : QWidget(editor)
  , m_Editor(editor)
  , m_Handle(handle)
  , m_FirstId(firstId)
  , m_SecondId(secondId)
  , m_Proximity(proximity)
{
  if (nullptr == editor)
  {
    mitkThrow() << "QmitkMxNLinkSeamWidget: editor must not be null.";
  }
  if (nullptr == handle)
  {
    mitkThrow() << "QmitkMxNLinkSeamWidget: splitter handle must not be null.";
  }
  if (nullptr == proximity)
  {
    mitkThrow() << "QmitkMxNLinkSeamWidget: proximity controller must not be null.";
  }

  this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  this->setFocusPolicy(Qt::NoFocus);

  // The seam follows the handle; the handle also feeds the reveal.
  handle->installEventFilter(this);
  proximity->AddEventSource(handle);
  proximity->AddEventSource(this);

  m_Region = proximity->RegisterRegion(
    [this]()
    {
      // The reveal anchors on the seam line itself, not the inflated
      // widget rect (which only exists to give the pill room).
      return m_Handle.isNull() || m_Editor.isNull()
        ? QRect()
        : QRect(m_Handle->mapTo(m_Editor, QPoint(0, 0)), m_Handle->size());
    },
    SeamActivationDistance);
  connect(proximity, &QmitkRenderWindowProximity::StateChanged,
          this, &QmitkMxNLinkSeamWidget::OnProximityStateChanged);

  // Link changes repaint the segments/chips; the widget set itself is
  // rebuilt by the editor on layout changes.
  connect(editor, &QmitkMxNMultiWidget::SyncLinksChanged,
          this, QOverload<>::of(&QWidget::update));

  this->FollowHandle();
  this->show();
  this->raise();
}

QmitkMxNLinkSeamWidget::~QmitkMxNLinkSeamWidget()
{
  if (!m_Proximity.isNull() && m_Region >= 0)
  {
    m_Proximity->UnregisterRegion(m_Region);
  }
}

bool QmitkMxNLinkSeamWidget::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == m_Handle
      && (event->type() == QEvent::Resize || event->type() == QEvent::Move
          || event->type() == QEvent::Show))
  {
    this->FollowHandle();
  }
  return QWidget::eventFilter(watched, event);
}

void QmitkMxNLinkSeamWidget::FollowHandle()
{
  if (m_Handle.isNull() || m_Editor.isNull())
  {
    return;
  }

  const QRect handleRect(m_Handle->mapTo(m_Editor, QPoint(0, 0)), m_Handle->size());

  // Inflate perpendicular to the seam so the expanded pill has room; along
  // the seam the handle's own extent is kept.
  const int pillHalf = (ChipSize + 2 * PillPadding) / 2 + 2;
  const int reach = std::max(pillHalf, 110);
  QRect widgetRect = m_Handle->orientation() == Qt::Horizontal
    ? handleRect.adjusted(-reach, 0, reach, 0)   // vertical seam line
    : handleRect.adjusted(0, -reach, 0, reach);  // horizontal seam line

  this->setGeometry(widgetRect.intersected(m_Editor->rect()));
  this->raise();
  this->UpdateInteractivity();
  this->update();
}

bool QmitkMxNLinkSeamWidget::IsRevealed() const
{
  return m_State == QmitkRenderWindowProximity::State::Active;
}

QRect QmitkMxNLinkSeamWidget::PillRect() const
{
  const auto chips = this->ChipLayout();
  if (chips.empty())
  {
    return QRect();
  }

  QRect bounds = chips.front().rect;
  for (const auto& chip : chips)
  {
    bounds = bounds.united(chip.rect);
  }
  return bounds.adjusted(-PillPadding, -PillPadding, PillPadding, PillPadding);
}

std::vector<QmitkMxNLinkSeamWidget::Chip> QmitkMxNLinkSeamWidget::ChipLayout() const
{
  std::vector<Chip> chips;

  const int chipCount = static_cast<int>(QmitkMxNAllSyncDimensions.size()) + 2;
  const int totalWidth = chipCount * ChipSize + (chipCount - 1) * ChipSpacing;
  const QPoint center = this->rect().center();
  int x = center.x() - totalWidth / 2;
  const int y = center.y() - ChipSize / 2;

  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    Chip chip;
    chip.rect = QRect(x, y, ChipSize, ChipSize);
    chip.dimension = dimension;
    chips.push_back(chip);
    x += ChipSize + ChipSpacing;
  }

  Chip bundle;
  bundle.rect = QRect(x, y, ChipSize, ChipSize);
  bundle.isBundle = true;
  chips.push_back(bundle);
  x += ChipSize + ChipSpacing;

  Chip editorHook;
  editorHook.rect = QRect(x, y, ChipSize, ChipSize);
  editorHook.isEditorHook = true;
  chips.push_back(editorHook);

  return chips;
}

std::optional<std::string> QmitkMxNLinkSeamWidget::LinkedGroup(
  const QString& windowId, QmitkMxNSyncDimension dimension) const
{
  if (m_Editor.isNull())
  {
    return std::nullopt;
  }
  const auto link = m_Editor->GetSyncLink(windowId, dimension);
  return link.has_value() ? std::optional<std::string>(link->group) : std::nullopt;
}

QmitkMxNLinkSeamWidget::PairState QmitkMxNLinkSeamWidget::GetDimensionState(
  QmitkMxNSyncDimension dimension) const
{
  const auto first = this->LinkedGroup(m_FirstId, dimension);
  const auto second = this->LinkedGroup(m_SecondId, dimension);

  if (first.has_value() && second.has_value())
  {
    return *first == *second ? PairState::Shared : PairState::Mixed;
  }
  return first.has_value() || second.has_value() ? PairState::OneSided : PairState::None;
}

std::vector<std::string> QmitkMxNLinkSeamWidget::GetSharedGroups() const
{
  std::vector<std::string> groups;
  for (const auto dimension : QmitkMxNAllSyncDimensions)
  {
    if (this->GetDimensionState(dimension) != PairState::Shared)
    {
      continue;
    }
    const auto group = *this->LinkedGroup(m_FirstId, dimension);
    if (std::find(groups.begin(), groups.end(), group) == groups.end())
    {
      groups.push_back(group);
    }
  }
  return groups;
}

std::string QmitkMxNLinkSeamWidget::MakePairGroup()
{
  const auto index = m_Editor->NextFreeSyncGroupIndex();
  m_Editor->AddSynchronizationGroup(index);
  return m_Editor->GetSyncGroupDisplayName(index).toStdString();
}

void QmitkMxNLinkSeamWidget::ToggleDimension(QmitkMxNSyncDimension dimension)
{
  if (m_Editor.isNull())
  {
    return;
  }

  try
  {
    switch (this->GetDimensionState(dimension))
    {
      case PairState::None:
      {
        const auto group = this->MakePairGroup();
        m_Editor->SetSyncLink(m_FirstId, dimension, group);
        m_Editor->SetSyncLink(m_SecondId, dimension, group);
        break;
      }
      case PairState::OneSided:
      {
        const auto first = this->LinkedGroup(m_FirstId, dimension);
        if (first.has_value())
        {
          m_Editor->SetSyncLink(m_SecondId, dimension, *first);
        }
        else
        {
          m_Editor->SetSyncLink(m_FirstId, dimension, *this->LinkedGroup(m_SecondId, dimension));
        }
        break;
      }
      case PairState::Shared:
        m_Editor->ClearSyncLink(m_SecondId, dimension);
        break;
      case PairState::Mixed:
        // Deliberately inert: resolving cross-group coupling is the layout
        // editor's job; the seam must never silently rip a cell out of a
        // group the user assembled there.
        break;
    }
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Seam: link change ignored: " << e.GetDescription();
  }

  this->update();
}

void QmitkMxNLinkSeamWidget::ToggleNavigationBundle()
{
  if (m_Editor.isNull())
  {
    return;
  }

  const bool allShared = std::all_of(NavigationBundle.begin(), NavigationBundle.end(),
    [this](QmitkMxNSyncDimension dimension)
    {
      return this->GetDimensionState(dimension) == PairState::Shared;
    });

  try
  {
    if (allShared)
    {
      for (const auto dimension : NavigationBundle)
      {
        m_Editor->ClearSyncLink(m_SecondId, dimension);
      }
    }
    else
    {
      // Prefer a navigation group either cell already has, so the bundle
      // extends existing coupling instead of fragmenting it.
      std::optional<std::string> group;
      for (const auto dimension : NavigationBundle)
      {
        group = this->LinkedGroup(m_FirstId, dimension);
        if (!group.has_value())
        {
          group = this->LinkedGroup(m_SecondId, dimension);
        }
        if (group.has_value())
        {
          break;
        }
      }
      if (!group.has_value())
      {
        group = this->MakePairGroup();
      }
      for (const auto dimension : NavigationBundle)
      {
        m_Editor->SetSyncLink(m_FirstId, dimension, *group);
        m_Editor->SetSyncLink(m_SecondId, dimension, *group);
      }
    }
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Seam: navigation bundle ignored: " << e.GetDescription();
  }

  this->update();
}

void QmitkMxNLinkSeamWidget::OnProximityStateChanged(QmitkRenderWindowProximity::RegionId id,
                                                     QmitkRenderWindowProximity::State state)
{
  if (id != m_Region)
  {
    return;
  }

  m_State = state;
  this->UpdateInteractivity();
  this->update();
}

void QmitkMxNLinkSeamWidget::UpdateInteractivity()
{
  if (!this->IsRevealed())
  {
    this->clearMask();
    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    return;
  }

  // Input (and painting) only over the pill: everything else on the seam
  // stays the splitter handle's, so resizing keeps working while revealed.
  this->setMask(this->PillRect());
  this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
  this->setMouseTracking(true);
}

void QmitkMxNLinkSeamWidget::paintEvent(QPaintEvent* /*event*/)
{
  if (m_Editor.isNull() || m_Handle.isNull())
  {
    return;
  }

  QPainter painter(this);

  if (!this->IsRevealed())
  {
    // Idle: one short hue segment per shared group, centered on the seam.
    // An unlinked pair paints nothing - the layout looks exactly like an
    // editor without seams.
    const auto groups = this->GetSharedGroups();
    if (groups.empty())
    {
      return;
    }

    const QRect handleRect(m_Handle->mapTo(m_Editor, QPoint(0, 0)), m_Handle->size());
    const QRect seamRect(this->mapFromParent(handleRect.topLeft()), handleRect.size());
    const bool verticalSeam = m_Handle->orientation() == Qt::Horizontal;
    const int seamLength = verticalSeam ? seamRect.height() : seamRect.width();
    const int segmentLength = seamLength / 4;
    const int count = static_cast<int>(groups.size());
    int position = (seamLength - count * segmentLength) / 2;

    for (const auto& group : groups)
    {
      QColor color(NeutralChip);
      try
      {
        color = m_Editor->GetSyncGroupColor(group);
      }
      catch (const mitk::Exception&)
      {
      }

      const QRect segment = verticalSeam
        ? QRect(seamRect.center().x() - SeamSegmentWidth / 2, seamRect.top() + position,
                SeamSegmentWidth, segmentLength)
        : QRect(seamRect.left() + position, seamRect.center().y() - SeamSegmentWidth / 2,
                segmentLength, SeamSegmentWidth);
      painter.fillRect(segment, color);
      position += segmentLength;
    }
    return;
  }

  // Revealed: the chip pill.
  const QRect pill = this->PillRect();
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setPen(Qt::NoPen);
  painter.setBrush(PillScrim);
  painter.drawRoundedRect(pill, 4, 4);
  painter.setRenderHint(QPainter::Antialiasing, false);

  for (const auto& chip : this->ChipLayout())
  {
    if (chip.isBundle)
    {
      const bool allShared = std::all_of(NavigationBundle.begin(), NavigationBundle.end(),
        [this](QmitkMxNSyncDimension dimension)
        {
          return this->GetDimensionState(dimension) == PairState::Shared;
        });
      painter.setPen(QPen(allShared ? ActiveGlyph : NeutralChip, 1));
      painter.drawRect(chip.rect.adjusted(0, 0, -1, -1));
      painter.drawText(chip.rect, Qt::AlignCenter, QStringLiteral("Nav"));
      continue;
    }
    if (chip.isEditorHook)
    {
      painter.setPen(QPen(NeutralChip, 1));
      painter.drawRect(chip.rect.adjusted(0, 0, -1, -1));
      painter.drawText(chip.rect, Qt::AlignCenter, QStringLiteral("..."));
      continue;
    }

    const auto state = this->GetDimensionState(chip.dimension);
    QColor hue(NeutralChip);
    if (state == PairState::Shared || state == PairState::OneSided)
    {
      const auto group = this->LinkedGroup(m_FirstId, chip.dimension).has_value()
        ? this->LinkedGroup(m_FirstId, chip.dimension)
        : this->LinkedGroup(m_SecondId, chip.dimension);
      try
      {
        hue = m_Editor->GetSyncGroupColor(*group);
      }
      catch (const mitk::Exception&)
      {
      }
    }

    switch (state)
    {
      case PairState::Shared:
        painter.fillRect(chip.rect, hue);
        DrawDimensionGlyph(painter, chip.rect, chip.dimension, QColor(0, 0, 0, 200));
        break;
      case PairState::OneSided:
        painter.setPen(QPen(hue, 1));
        painter.drawRect(chip.rect.adjusted(0, 0, -1, -1));
        DrawDimensionGlyph(painter, chip.rect, chip.dimension, hue);
        break;
      case PairState::None:
        painter.setPen(QPen(NeutralChip, 1));
        painter.drawRect(chip.rect.adjusted(0, 0, -1, -1));
        DrawDimensionGlyph(painter, chip.rect, chip.dimension, NeutralChip);
        break;
      case PairState::Mixed:
      {
        painter.setPen(QPen(QColor(128, 128, 128, 160), 1, Qt::DashLine));
        painter.drawRect(chip.rect.adjusted(0, 0, -1, -1));
        DrawDimensionGlyph(painter, chip.rect, chip.dimension, QColor(128, 128, 128, 160));
        break;
      }
    }
  }
}

void QmitkMxNLinkSeamWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton || !this->IsRevealed())
  {
    event->ignore();
    return;
  }

  for (const auto& chip : this->ChipLayout())
  {
    if (!chip.rect.contains(event->pos()))
    {
      continue;
    }

    event->accept();
    if (chip.isEditorHook)
    {
      if (!m_Editor.isNull())
      {
        m_Editor->RequestLayoutEditor();
      }
    }
    else if (chip.isBundle)
    {
      this->ToggleNavigationBundle();
    }
    else
    {
      this->ToggleDimension(chip.dimension);
    }
    return;
  }

  event->ignore();
}

bool QmitkMxNLinkSeamWidget::event(QEvent* event)
{
  if (event->type() == QEvent::ToolTip && this->IsRevealed())
  {
    const auto* helpEvent = static_cast<QHelpEvent*>(event);
    for (const auto& chip : this->ChipLayout())
    {
      if (!chip.rect.contains(helpEvent->pos()))
      {
        continue;
      }
      QString text;
      if (chip.isEditorHook)
      {
        text = tr("Open the MxN layout editor");
      }
      else if (chip.isBundle)
      {
        text = tr("Couple/uncouple this pair on the navigation bundle "
                  "(pan, zoom, slice, crosshair)");
      }
      else
      {
        const auto name = QString::fromUtf8(DimensionName(chip.dimension));
        switch (this->GetDimensionState(chip.dimension))
        {
          case PairState::Shared:
            text = tr("%1: coupled; click to uncouple this pair").arg(name);
            break;
          case PairState::OneSided:
            text = tr("%1: click to couple this pair").arg(name);
            break;
          case PairState::None:
            text = tr("%1: click to couple this pair (creates a new group)").arg(name);
            break;
          case PairState::Mixed:
            text = tr("%1: the two windows are in different groups; resolve in the "
                      "layout editor").arg(name);
            break;
        }
      }
      QToolTip::showText(helpEvent->globalPos(), text, this);
      return true;
    }
  }
  return QWidget::event(event);
}
