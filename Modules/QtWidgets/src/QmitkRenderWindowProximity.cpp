/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderWindowProximity.h"

#include <mitkExceptionMacro.h>

#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QWidget>

#include <algorithm>

namespace
{
  int SquaredDistanceToRect(const QPoint& point, const QRect& rect)
  {
    const int dx = std::max({0, rect.left() - point.x(), point.x() - rect.right()});
    const int dy = std::max({0, rect.top() - point.y(), point.y() - rect.bottom()});
    return dx * dx + dy * dy;
  }
}

QmitkRenderWindowProximity::QmitkRenderWindowProximity(QWidget* cell, QObject* parent)
  : QObject(parent)
  , m_Cell(cell)
{
  if (nullptr == cell)
  {
    mitkThrow() << "No cell widget given to track the pointer over.";
  }

  cell->setMouseTracking(true);
  cell->installEventFilter(this);
}

QmitkRenderWindowProximity::~QmitkRenderWindowProximity()
{
}

void QmitkRenderWindowProximity::AddEventSource(QWidget* source)
{
  if (nullptr == source)
  {
    mitkThrow() << "Cannot add a null widget as a proximity event source.";
  }

  source->setMouseTracking(true);
  source->installEventFilter(this);
}

QmitkRenderWindowProximity::RegionId QmitkRenderWindowProximity::RegisterRegion(
  std::function<QRect()> regionInCellCoords, int activationDistance)
{
  if (!regionInCellCoords)
  {
    mitkThrow() << "Cannot register a proximity region without a rectangle callback.";
  }
  if (activationDistance <= 0)
  {
    mitkThrow() << "Proximity activation distance must be positive (got "
                << activationDistance << ").";
  }

  const RegionId id = m_NextRegionId++;

  Region region;
  region.rectQuery = std::move(regionInCellCoords);
  region.activationDistance = activationDistance;
  region.collapseTimer = new QTimer(this);
  region.collapseTimer->setSingleShot(true);
  region.collapseTimer->setInterval(CollapseDelayMs);
  connect(region.collapseTimer, &QTimer::timeout, this, [this, id]() { this->OnCollapseTimeout(id); });

  auto [it, inserted] = m_Regions.emplace(id, std::move(region));
  this->EvaluateRegion(id, it->second);

  return id;
}

void QmitkRenderWindowProximity::UnregisterRegion(RegionId id)
{
  auto it = m_Regions.find(id);

  if (it == m_Regions.end())
  {
    mitkThrow() << "Cannot unregister unknown proximity region id " << id << ".";
  }

  it->second.collapseTimer->deleteLater();
  m_Regions.erase(it);
}

QmitkRenderWindowProximity::State QmitkRenderWindowProximity::GetRegionState(RegionId id) const
{
  auto it = m_Regions.find(id);

  if (it == m_Regions.end())
  {
    mitkThrow() << "Unknown proximity region id " << id << ".";
  }

  return it->second.state;
}

void QmitkRenderWindowProximity::SetSuppressed(bool suppressed)
{
  if (suppressed == m_Suppressed)
  {
    return;
  }

  m_Suppressed = suppressed;

  if (suppressed)
  {
    // Clean-view must take effect instantly, so bypass the collapse delay.
    for (auto& [id, region] : m_Regions)
    {
      region.collapseTimer->stop();

      if (region.state != State::Idle)
      {
        this->ApplyState(id, region, State::Idle);
      }
    }
  }
  else
  {
    this->EvaluateAllRegions();
  }
}

bool QmitkRenderWindowProximity::IsSuppressed() const
{
  return m_Suppressed;
}

void QmitkRenderWindowProximity::HandlePointerMoved(const QPoint& positionInCell, bool buttonsPressed)
{
  m_PointerPosition = positionInCell;
  m_PointerInside = true;
  m_ButtonsPressed = buttonsPressed;

  this->EvaluateAllRegions();
}

void QmitkRenderWindowProximity::HandlePointerLeft()
{
  m_PointerInside = false;

  this->EvaluateAllRegions();
}

bool QmitkRenderWindowProximity::eventFilter(QObject* watched, QEvent* event)
{
  // Only the cell and explicitly added sources are watched, so anything
  // arriving here is ours; it just needs resolving against the cell.
  if (m_Cell.isNull())
  {
    return false;
  }

  switch (event->type())
  {
    case QEvent::Resize:
      // Region rectangles are queried lazily, but a stationary pointer gets
      // no move event when the cell resizes under it; re-evaluate explicitly.
      if (watched == m_Cell && m_PointerInside)
      {
        this->EvaluateAllRegions();
      }
      return false;

    case QEvent::Leave:
      // Qt delivers enter/leave along the parent chain: the cell frame gets
      // its Leave exactly when the pointer exits the cell subtree, while a
      // Leave on a source may just mean crossing into a sibling child.
      if (watched == m_Cell)
      {
        this->HandlePointerLeft();
      }
      return false;

    case QEvent::MouseMove:
    case QEvent::Enter:
    {
      const auto* pointerEvent = static_cast<QSinglePointEvent*>(event);
      const auto globalPosition = pointerEvent->globalPosition().toPoint();
      const auto positionInCell = m_Cell->mapFromGlobal(globalPosition);

      if (m_Cell->rect().contains(positionInCell))
      {
        this->HandlePointerMoved(positionInCell, pointerEvent->buttons() != Qt::NoButton);
      }
      else if (m_PointerInside)
      {
        // A drag's implicit grab keeps a source's moves coming after the
        // pointer has left the cell.
        this->HandlePointerLeft();
      }
      return false;
    }

    default:
      return false;
  }
}

QmitkRenderWindowProximity::State QmitkRenderWindowProximity::ComputeState(const Region& region) const
{
  if (m_Suppressed || !m_PointerInside)
  {
    return State::Idle;
  }

  const QRect rect = region.rectQuery();

  if (rect.isValid())
  {
    // Once active, the region stays active through the hysteresis band, so a
    // pointer resting near the threshold cannot flap the state.
    const int threshold = region.state == State::Active
      ? region.activationDistance + HysteresisBand
      : region.activationDistance;

    if (SquaredDistanceToRect(m_PointerPosition, rect) <= threshold * threshold)
    {
      // Reveal on a bare hover only: while a mouse button is held, a region
      // that is not already revealed stays at Hint, so the frame never pops in
      // mid-gesture (drawing, crosshairing or windowing over the image). An
      // already-active region is left alone - a drag begun on the furniture
      // itself must keep it up.
      if (m_ButtonsPressed && region.state != State::Active)
      {
        return State::Hint;
      }
      return State::Active;
    }
  }

  return State::Hint;
}

void QmitkRenderWindowProximity::EvaluateRegion(RegionId id, Region& region)
{
  const State computed = this->ComputeState(region);

  if (static_cast<int>(computed) >= static_cast<int>(region.state))
  {
    region.collapseTimer->stop();

    if (computed != region.state)
    {
      this->ApplyState(id, region, computed);
    }
  }
  else if (!region.collapseTimer->isActive())
  {
    region.collapseTimer->start();
  }
}

void QmitkRenderWindowProximity::EvaluateAllRegions()
{
  for (auto& [id, region] : m_Regions)
  {
    this->EvaluateRegion(id, region);
  }
}

void QmitkRenderWindowProximity::ApplyState(RegionId id, Region& region, State state)
{
  region.state = state;
  emit StateChanged(id, state);
}

void QmitkRenderWindowProximity::OnCollapseTimeout(RegionId id)
{
  auto it = m_Regions.find(id);

  if (it == m_Regions.end())
  {
    return;
  }

  const State computed = this->ComputeState(it->second);

  if (computed != it->second.state)
  {
    this->ApplyState(id, it->second, computed);
  }
}
