/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNCellOverlay.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkAnatomicalPlanes.h>
#include <mitkBaseRenderer.h>
#include <mitkDisplayActionEvents.h>
#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <mitkInteractionEvent.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkTimeNavigationController.h>

#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkLookupTable.h>
#include <vtkRenderWindow.h>

#include <QApplication>
#include <QContextMenuEvent>
#include <QDoubleSpinBox>
#include <QFontMetrics>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPolygon>
#include <QPropertyAnimation>
#include <QSpinBox>
#include <QStyle>
#include <QTimer>

#include <array>
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
  constexpr int PassiveRibbonWidth = 2;
  constexpr int ActiveRibbonWidth = 14;
  constexpr int RibbonEndHandleHeight = 16;
  constexpr int ReadoutMargin = 5;        // tight edge padding to spare canvas
  constexpr int ChipSize = 14;
  constexpr int EdgeStripThickness = 20;
  constexpr int EdgeActivationDistance = 16;    // top-strip reveal floor: a tighter approach than the furniture
  constexpr int FurnitureActivationFloor = 48;  // fixed reveal floor for the painted furniture (small cells)
  constexpr double CanvasReactiveFraction = 0.10;  // reveal within x% of the render extent -> inner (100-2x)% stays silent
  constexpr int TopStripHeight = 4;
  constexpr int SliceTickHeight = 8;
  constexpr int LineGap = 1;              // between the two bottom-left lines
  constexpr int TickScaleWidth = 34;      // labels left of the active colorbar
  constexpr int RevealSlideOffset = 6;    // px the reveal furniture slides in
  constexpr int TimeTriangleHalfWidth = 3;
  constexpr int NavRowHeight = 16;        // one navigator slider row
  constexpr int NavRowGap = 4;
  constexpr int NavLabelWidth = 78;       // left label column of a navigator row
  constexpr int NavKnobRadius = 5;
  constexpr int NavTrackThickness = 2;
  // The active colorbar does not span the whole edge: it is inset top and
  // bottom so the range labels never crowd the top chrome or the W/L readout,
  // and so the colormap chip has room below it.
  constexpr int ColorbarInsetTop = 30;
  constexpr int ColorbarInsetBottom = 46;

  const QColor IdleText(255, 255, 255, 140);    // 55 % white
  const QColor ActiveText(255, 255, 255, 216);  // 85 % white

  /** \brief Reveal margin for a furniture edge: a fraction of the render
   *         extent perpendicular to that edge (so it scales with the canvas),
   *         never below a fixed floor for small cells. */
  int ReactiveDistance(int renderExtentPx, int floorPx)
  {
    return std::max(floorPx, qRound(CanvasReactiveFraction * renderExtentPx));
  }

  /** \brief The one peripheral-readout font: a single size + tabular numerals
   *         for every readout (plane, slice, W/L, colorbar ticks, navigator),
   *         so the instrument panel reads as one type scale. */
  QFont ReadoutFont(const QFont& base)
  {
    QFont font(base);
    font.setPointSize(9);
    // Tabular numerals keep the readouts from jittering while values change.
    font.setFeature(QFont::Tag("tnum"), 1);
    return font;
  }

  QString FormatValue(double value)
  {
    return QString::number(std::llround(value));
  }

  /** \brief A copy of 'base' with its alpha scaled by 'factor' (0..1). */
  QColor Faded(const QColor& base, qreal factor)
  {
    QColor color(base);
    color.setAlpha(static_cast<int>(std::round(base.alpha() * std::clamp(factor, 0.0, 1.0))));
    return color;
  }

  /** \brief The clock glyph preceding the 4D time readout: a ring with two
   *         hands, drawn to fit 'box'. Antialiasing is expected to be on. */
  void DrawClockGlyph(QPainter& painter, const QRect& box, const QColor& color)
  {
    QPen pen(color, 1.2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    const QRect ring = box.adjusted(1, 1, -1, -1);
    painter.drawEllipse(ring);
    const QPointF center = QRectF(ring).center();
    painter.drawLine(center, center + QPointF(0, -ring.height() * 0.30));
    painter.drawLine(center, center + QPointF(ring.width() * 0.24, 0));
  }
}

QmitkMxNCellOverlay::QmitkMxNCellOverlay(QmitkRenderWindowWidget* cell,
                                         QmitkMxNMultiWidget* editor,
                                         QmitkRenderWindowProximity* proximity)
  : QmitkOverlayWidget(cell)
  , m_Cell(cell)
  , m_Editor(editor)
  , m_Proximity(proximity)
{
  if (nullptr == cell)
  {
    mitkThrow() << "QmitkMxNCellOverlay: cell must not be null.";
  }
  if (nullptr == editor)
  {
    mitkThrow() << "QmitkMxNCellOverlay: editor must not be null.";
  }
  if (nullptr == proximity)
  {
    mitkThrow() << "QmitkMxNCellOverlay: proximity controller must not be null.";
  }
  if (nullptr == cell->GetRenderWindow() || nullptr == cell->GetRenderWindow()->GetVtkRenderWindow())
  {
    mitkThrow() << "QmitkMxNCellOverlay: the cell has no render window yet.";
  }

  this->setFocusPolicy(Qt::NoFocus);

  // While the overlay is interactive (masked), pointer events land here
  // instead of on the render window; feeding them back keeps the proximity
  // state alive while the pointer rests on the furniture itself.
  proximity->AddEventSource(this);

  // Each furniture edge reveals within a margin that scales with the render
  // window - a fraction of the extent perpendicular to that edge - so the inner
  // region of the canvas stays silent at any cell size. The right-edge colorbar
  // scales with the width; the bottom furniture and the top strip with the
  // height.
  m_RibbonRegion = proximity->RegisterRegion(
    [this]() { return this->RibbonRect(); },
    [this]() { return ReactiveDistance(this->RenderWindowRect().width(), FurnitureActivationFloor); });
  m_WindowLevelRegion = proximity->RegisterRegion(
    [this]() { return this->WindowLevelRect(); },
    [this]() { return ReactiveDistance(this->RenderWindowRect().height(), FurnitureActivationFloor); });
  // The plane label is a control (click to reorient); it highlights and takes
  // input when the pointer is near it, like the W/L readout.
  m_PlaneLabelRegion = proximity->RegisterRegion(
    [this]() { return this->PlaneLabelRect(); },
    [this]() { return ReactiveDistance(this->RenderWindowRect().height(), FurnitureActivationFloor); });
  // The bottom region is the navigator band (the painted sliders); it goes
  // Active as the pointer approaches, enabling the drag and sharpening it.
  m_BottomRegion = proximity->RegisterRegion(
    [this]() { return this->NavigatorBandRect(); },
    [this]() { return ReactiveDistance(this->RenderWindowRect().height(), FurnitureActivationFloor); });
  m_TopRegion = proximity->RegisterRegion(
    [this]() { return this->TopStripRect(); },
    [this]() { return ReactiveDistance(this->RenderWindowRect().height(), EdgeActivationDistance); });
  connect(proximity, &QmitkRenderWindowProximity::StateChanged,
          this, &QmitkMxNCellOverlay::OnProximityStateChanged);

  // The group-identity dot (and the colorbar's level-marker hue) are resolved
  // live from the group state at paint time, but a grouping change in the
  // layout editor need not trigger a render - so repaint on the editor's
  // link-change signal to keep them current.
  connect(m_Editor, &QmitkMxNMultiWidget::SyncLinksChanged, this, [this]()
  {
    this->UpdateInteractivity();
    this->update();
  });

  // The interactive furniture reveals as one coordinated frame (opacity +
  // offset), not region by region; the proximity controller supplies the
  // collapse delay and hysteresis, this only the easing.
  m_RevealAnimation = new QPropertyAnimation(this, "revealProgress", this);
  m_RevealAnimation->setDuration(QmitkRenderWindowProximity::RevealDurationMs);

  // Context-menu handling needs the render window's own mouse stream (the
  // overlay is transparent there); the base class already filters the cell.
  cell->GetRenderWindow()->installEventFilter(this);

  // Keep a reference of our own: the observer must be removable in the
  // destructor regardless of the Qt child destruction order.
  m_VtkRenderWindow = cell->GetRenderWindow()->GetVtkRenderWindow();

  auto callback = vtkSmartPointer<vtkCallbackCommand>::New();
  callback->SetClientData(this);
  callback->SetCallback(&QmitkMxNCellOverlay::OnVtkRenderEnd);
  m_VtkObserverTag = m_VtkRenderWindow->AddObserver(vtkCommand::EndEvent, callback);

  this->ScheduleValueRefresh();
}

QmitkMxNCellOverlay::~QmitkMxNCellOverlay()
{
  if (nullptr != m_VtkRenderWindow)
  {
    m_VtkRenderWindow->RemoveObserver(m_VtkObserverTag);
  }
}

void QmitkMxNCellOverlay::SetReadoutVisible(bool visible)
{
  if (visible == m_ReadoutVisible)
  {
    return;
  }

  m_ReadoutVisible = visible;
  this->UpdateInteractivity();
  this->update();
}

void QmitkMxNCellOverlay::SetCleanView(bool cleanView)
{
  if (cleanView == m_CleanView)
  {
    return;
  }

  m_CleanView = cleanView;
  this->UpdateInteractivity();
  this->update();
}

QRect QmitkMxNCellOverlay::RenderWindowRect() const
{
  auto* renderWindow = m_Cell->GetRenderWindow();
  return nullptr != renderWindow ? renderWindow->geometry() : QRect();
}

QRect QmitkMxNCellOverlay::RibbonRect() const
{
  const QRect area = this->RenderWindowRect();
  if (!area.isValid())
  {
    return QRect();
  }

  // The interactive colorbar (drag / tick scale / chip anchor) is the revealed
  // wide bar, inset top and bottom so its range labels clear the top chrome
  // and the W/L readout and the colormap chip has room below it. The passive
  // idle strip is painted separately by paintEvent as the reveal animates.
  return QRect(area.right() - ActiveRibbonWidth + 1, area.top() + ColorbarInsetTop,
               ActiveRibbonWidth, area.height() - ColorbarInsetTop - ColorbarInsetBottom);
}

QRect QmitkMxNCellOverlay::SliceReadoutRect() const
{
  const QRect area = this->RenderWindowRect();
  if (!area.isValid())
  {
    return QRect();
  }

  if (m_SliceSteps == 0)
  {
    return QRect();
  }

  const QFontMetrics metrics(ReadoutFont(this->font()));
  const int lineHeight = metrics.height() + 4;

  int width = metrics.horizontalAdvance(QStringLiteral("%1/%2").arg(m_SlicePosition + 1).arg(m_SliceSteps));
  if (m_TimeSteps > 1)
  {
    width += ReadoutMargin + lineHeight
      + metrics.horizontalAdvance(QStringLiteral(" %1/%2").arg(m_TimePosition + 1).arg(m_TimeSteps));
  }

  return QRect(area.left() + ReadoutMargin, area.bottom() - ReadoutMargin - lineHeight, width, lineHeight);
}

QRect QmitkMxNCellOverlay::PlaneLabelRect() const
{
  const QRect area = this->RenderWindowRect();
  const QString planeLabel = this->ResolvePlaneLabel();
  if (!area.isValid() || planeLabel.isEmpty())
  {
    return QRect();
  }

  const QFontMetrics metrics(ReadoutFont(this->font()));
  const int lineHeight = metrics.height() + 4;
  const int width = metrics.horizontalAdvance(planeLabel) + ReadoutMargin;

  // Line 1 sits directly above the slice line (line 2), whether or not the
  // slice line is currently populated, so the two always share the corner.
  const QRect below = this->SliceReadoutRect();
  const int bottom = (below.isValid() ? below.top() : area.bottom() - ReadoutMargin) - LineGap;
  return QRect(area.left() + ReadoutMargin, bottom - lineHeight, width, lineHeight);
}

QRect QmitkMxNCellOverlay::WindowLevelRect() const
{
  const QRect area = this->RenderWindowRect();
  if (!area.isValid() || !m_HasLevelWindow)
  {
    return QRect();
  }

  const QFontMetrics metrics(ReadoutFont(this->font()));
  const QString text = QStringLiteral("W %1 L %2")
    .arg(FormatValue(m_LevelWindow.GetWindow()), FormatValue(m_LevelWindow.GetLevel()));
  const int width = metrics.horizontalAdvance(text) + ReadoutMargin;
  const int lineHeight = metrics.height() + 4;

  // Bottom-right, immediately left of the colorbar, so value and legend read
  // as one intensity cluster.
  const int right = area.right() - ActiveRibbonWidth - ReadoutMargin;
  return QRect(right - width, area.bottom() - ReadoutMargin - lineHeight, width, lineHeight);
}

QRect QmitkMxNCellOverlay::ColormapChipRect() const
{
  const QRect ribbon = this->RibbonRect();
  if (!ribbon.isValid())
  {
    return QRect();
  }

  // In the room the bottom inset leaves beneath the shortened bar, so it never
  // overlaps the range labels above or the W/L readout to its left.
  return QRect(ribbon.left(), ribbon.bottom() + ReadoutMargin, ChipSize, ChipSize);
}

QRect QmitkMxNCellOverlay::TopStripRect() const
{
  return QRect(0, 0, this->width(), EdgeStripThickness);
}

bool QmitkMxNCellOverlay::IsPassiveVisible(bool honorReadoutPreference) const
{
  if (m_CleanView)
  {
    return false;
  }
  // The always-on readouts show whenever the frame is not clean; the
  // level/window readout additionally obeys its preference, following the
  // reveal (frame progress) when the preference has it off.
  return !honorReadoutPreference || m_ReadoutVisible || m_RevealProgress > 0.0;
}

void QmitkMxNCellOverlay::OnProximityStateChanged(QmitkRenderWindowProximity::RegionId id,
                                                  QmitkRenderWindowProximity::State state)
{
  if (id == m_RibbonRegion)
  {
    m_RibbonState = state;
  }
  else if (id == m_WindowLevelRegion)
  {
    m_WindowLevelState = state;
  }
  else if (id == m_PlaneLabelRegion)
  {
    m_PlaneLabelState = state;
  }
  else if (id == m_BottomRegion)
  {
    m_BottomState = state;
  }
  else if (id == m_TopRegion)
  {
    m_TopState = state;
  }
  else
  {
    return;
  }

  this->UpdateReveal();
  this->UpdateInteractivity();
  this->update();
}

bool QmitkMxNCellOverlay::IsFrameRevealed() const
{
  using State = QmitkRenderWindowProximity::State;
  // Reveal only in the proximity hotzone of the furniture / frame edges (a
  // region gone Active), not merely while the pointer is somewhere in the
  // cell - the central image stays uncovered.
  return !m_CleanView
    && (m_RibbonState == State::Active || m_WindowLevelState == State::Active
        || m_PlaneLabelState == State::Active || m_BottomState == State::Active
        || m_TopState == State::Active);
}

bool QmitkMxNCellOverlay::AnimationsEnabled() const
{
  // The one portable "reduced motion" signal Qt exposes: a style that wants
  // no animation reports a zero widget-animation duration.
  return this->style()->styleHint(QStyle::SH_Widget_Animation_Duration, nullptr, this) > 0;
}

void QmitkMxNCellOverlay::UpdateReveal()
{
  const bool revealed = this->IsFrameRevealed();
  if (revealed == m_FrameRevealed)
  {
    return;
  }
  m_FrameRevealed = revealed;

  // The auto-hidden utility strip is part of the one frame: it reveals and
  // collapses together with the painted furniture, not on its own top-edge
  // proximity.
  m_Cell->ShowUtilityWidget(revealed);

  const qreal target = revealed ? 1.0 : 0.0;
  if (m_RevealAnimation.isNull() || !this->AnimationsEnabled())
  {
    this->SetRevealProgress(target);
    return;
  }

  m_RevealAnimation->stop();
  m_RevealAnimation->setStartValue(m_RevealProgress);
  m_RevealAnimation->setEndValue(target);
  m_RevealAnimation->start();
}

qreal QmitkMxNCellOverlay::RevealProgress() const
{
  return m_RevealProgress;
}

void QmitkMxNCellOverlay::SetRevealProgress(qreal progress)
{
  progress = std::clamp<qreal>(progress, 0.0, 1.0);
  if (qFuzzyCompare(progress, m_RevealProgress))
  {
    return;
  }
  m_RevealProgress = progress;
  this->UpdateInteractivity();
  this->update();
}

QString QmitkMxNCellOverlay::PlaneLabel() const
{
  return this->ResolvePlaneLabel();
}

QString QmitkMxNCellOverlay::ResolvePlaneLabel() const
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer)
  {
    return QString();
  }

  // The same view direction the utility-row combobox drives, so a
  // reorientation is reflected without a separate signal.
  if (auto* sliceNavigation = renderer->GetSliceNavigationController())
  {
    switch (sliceNavigation->GetDefaultViewDirection())
    {
      case mitk::AnatomicalPlane::Axial:    return QStringLiteral("Axial");
      case mitk::AnatomicalPlane::Coronal:  return QStringLiteral("Coronal");
      case mitk::AnatomicalPlane::Sagittal: return QStringLiteral("Sagittal");
      default: break;  // 'Original' has no fixed anatomical name
    }
  }
  return QString();
}

void QmitkMxNCellOverlay::SetNavigatorExpanded(bool expanded)
{
  if (expanded == m_NavigatorExpanded)
  {
    return;
  }
  m_NavigatorExpanded = expanded;
  this->UpdateInteractivity();
  this->update();
}

bool QmitkMxNCellOverlay::IsNavigatorExpanded() const
{
  return m_NavigatorExpanded;
}

mitk::Point3D QmitkMxNCellOverlay::CrosshairWorld() const
{
  // GetSelectedPosition logs and returns the origin for an unknown window
  // rather than throwing, which is a harmless fallback here.
  return m_Editor->GetSelectedPosition(m_Cell->GetWidgetName());
}

QString QmitkMxNCellOverlay::NavigatorDepthLabel() const
{
  // Just "Slice": the orientation is already shown by the bottom-left plane
  // label, so naming it again on the depth row would be redundant.
  return QStringLiteral("Slice");
}

bool QmitkMxNCellOverlay::NavigatorInPlaneState(mitk::Point3D& origin, mitk::Vector3D& rightUnit,
                                                mitk::Vector3D& upUnit, double& extentRight,
                                                double& extentUp, double& rightCoord,
                                                double& upCoord) const
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer)
  {
    return false;
  }
  const auto* plane = renderer->GetCurrentWorldPlaneGeometry();
  if (nullptr == plane)
  {
    return false;
  }

  origin = plane->GetOrigin();
  const mitk::Vector3D right = plane->GetAxisVector(0);
  const mitk::Vector3D up = plane->GetAxisVector(1);
  extentRight = right.GetNorm();
  extentUp = up.GetNorm();
  if (extentRight < 1e-6 || extentUp < 1e-6)
  {
    return false;
  }
  rightUnit = right / extentRight;
  upUnit = up / extentUp;

  // The crosshair projected onto the plane's own axes - oblique-safe, no
  // dependence on the world coordinate axes.
  const mitk::Vector3D fromOrigin = this->CrosshairWorld() - origin;
  rightCoord = fromOrigin * rightUnit;
  upCoord = fromOrigin * upUnit;
  return true;
}

std::vector<QmitkMxNCellOverlay::NavRow> QmitkMxNCellOverlay::NavigatorRows() const
{
  std::vector<NavRow> rows;
  const QRect band = this->NavigatorBandRect();
  if (!band.isValid())
  {
    return rows;
  }

  rows.push_back({ NavRow::Kind::Slice, this->NavigatorDepthLabel(), 0.0, {} });
  if (m_NavigatorExpanded)
  {
    rows.push_back({ NavRow::Kind::InPlaneRight, tr("Horiz."), 0.0, {} });
    rows.push_back({ NavRow::Kind::InPlaneUp, tr("Vert."), 0.0, {} });
  }
  if (m_TimeSteps > 1)
  {
    rows.push_back({ NavRow::Kind::Time, tr("Time"), 0.0, {} });
  }

  // In-plane state is only needed when there is an in-plane row.
  mitk::Point3D origin;
  mitk::Vector3D rightUnit, upUnit;
  double extentRight = 0.0, extentUp = 0.0, rightCoord = 0.0, upCoord = 0.0;
  const bool hasPlane = m_NavigatorExpanded
    && this->NavigatorInPlaneState(origin, rightUnit, upUnit, extentRight, extentUp, rightCoord, upCoord);

  const int trackLeft = band.left() + NavLabelWidth + NavRowGap;
  const int trackWidth = std::max(1, band.right() - trackLeft);
  for (std::size_t i = 0; i < rows.size(); ++i)
  {
    const int rowTop = band.top() + static_cast<int>(i) * (NavRowHeight + NavRowGap);
    rows[i].track = QRect(trackLeft, rowTop, trackWidth, NavRowHeight);

    double normalized = 0.0;
    switch (rows[i].kind)
    {
      case NavRow::Kind::Slice:
        normalized = m_SliceSteps > 1
          ? static_cast<double>(m_SlicePosition) / (m_SliceSteps - 1) : 0.0;
        break;
      case NavRow::Kind::Time:
        normalized = m_TimeSteps > 1
          ? static_cast<double>(m_TimePosition) / (m_TimeSteps - 1) : 0.0;
        break;
      case NavRow::Kind::InPlaneRight:
        normalized = hasPlane ? rightCoord / extentRight : 0.0;
        break;
      case NavRow::Kind::InPlaneUp:
        normalized = hasPlane ? upCoord / extentUp : 0.0;
        break;
    }
    rows[i].normalized = std::clamp(normalized, 0.0, 1.0);
  }
  return rows;
}

QRect QmitkMxNCellOverlay::NavigatorBandRect() const
{
  const QRect area = this->RenderWindowRect();
  if (!area.isValid())
  {
    return QRect();
  }

  int rowCount = m_NavigatorExpanded ? 3 : 1;  // depth [+ H + V]
  if (m_TimeSteps > 1)
  {
    ++rowCount;  // time row
  }

  const QFontMetrics metrics(ReadoutFont(this->font()));
  const int readoutLineHeight = metrics.height() + 4;
  // Clear the two always-on bottom-left readout lines and the hairline.
  const int passiveBottom = ReadoutMargin + 2 * readoutLineHeight + LineGap;
  const int coordExtra = m_NavigatorExpanded ? (NavRowGap + NavRowHeight) : 0;
  const int bandHeight = rowCount * NavRowHeight + (rowCount - 1) * NavRowGap + coordExtra;

  const int bandBottom = area.bottom() - passiveBottom - NavRowGap;
  const int left = area.left() + ReadoutMargin;
  const int right = area.right() - ActiveRibbonWidth - ReadoutMargin;
  if (right <= left)
  {
    return QRect();
  }
  return QRect(left, bandBottom - bandHeight, right - left, bandHeight);
}

QRect QmitkMxNCellOverlay::CoordinateLineRect() const
{
  if (!m_NavigatorExpanded)
  {
    return QRect();
  }
  const QRect band = this->NavigatorBandRect();
  if (!band.isValid())
  {
    return QRect();
  }
  return QRect(band.left(), band.bottom() - NavRowHeight, band.width(), NavRowHeight);
}

void QmitkMxNCellOverlay::NavigatorSetSlice(int position)
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer)
  {
    return;
  }
  auto* stepper = renderer->GetSliceNavigationController()->GetStepper();
  if (nullptr == stepper)
  {
    return;
  }
  const int delta = position - static_cast<int>(stepper->GetPos());
  if (0 == delta)
  {
    return;
  }
  // Go through the display-action broadcast so a slice-linked cell's group
  // follows; an unlinked cell is a singleton group and moves alone.
  auto interactionEvent = mitk::InteractionEvent::New(renderer);
  m_Editor->GetInteractionEventHandler()->InvokeEvent(
    mitk::DisplayScrollEvent(interactionEvent, delta, false));
}

void QmitkMxNCellOverlay::NavigatorSetCrosshair(const mitk::Point3D& worldPosition)
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer)
  {
    return;
  }
  // Same broadcast path: a crosshair-linked cell follows, an unlinked cell
  // moves alone.
  auto interactionEvent = mitk::InteractionEvent::New(renderer);
  m_Editor->GetInteractionEventHandler()->InvokeEvent(
    mitk::DisplaySetCrosshairEvent(interactionEvent, worldPosition));
}

void QmitkMxNCellOverlay::NavigatorSetVoxelIndex(const mitk::Point3D& voxelIndex)
{
  // Resolve the reference image live rather than from the render-end cache so
  // the index fields work independently of a pending refresh; the voxel index
  // is that of the cell's top image geometry.
  const auto node = this->ResolveTopImageNode();
  if (node.IsNull() || nullptr == node->GetData() || nullptr == node->GetData()->GetGeometry())
  {
    return;
  }
  mitk::Point3D world;
  node->GetData()->GetGeometry()->IndexToWorld(voxelIndex, world);
  this->NavigatorSetCrosshair(world);
}

void QmitkMxNCellOverlay::NavigatorMoveInPlane(double rightMm, double upMm)
{
  mitk::Point3D origin;
  mitk::Vector3D rightUnit, upUnit;
  double extentRight = 0.0, extentUp = 0.0, rightCoord = 0.0, upCoord = 0.0;
  if (!this->NavigatorInPlaneState(origin, rightUnit, upUnit, extentRight, extentUp, rightCoord, upCoord))
  {
    return;
  }
  mitk::Point3D world = this->CrosshairWorld();
  for (int axis = 0; axis < 3; ++axis)
  {
    world[axis] += rightMm * rightUnit[axis] + upMm * upUnit[axis];
  }
  this->NavigatorSetCrosshair(world);
}

void QmitkMxNCellOverlay::ApplyNavigatorRow(const NavRow& row, double normalized)
{
  normalized = std::clamp(normalized, 0.0, 1.0);
  switch (row.kind)
  {
    case NavRow::Kind::Slice:
      if (m_SliceSteps > 1)
      {
        this->NavigatorSetSlice(static_cast<int>(std::lround(normalized * (m_SliceSteps - 1))));
      }
      break;
    case NavRow::Kind::Time:
      if (m_TimeSteps > 1)
      {
        if (auto* timeNavigation = mitk::RenderingManager::GetInstance()->GetTimeNavigationController())
        {
          if (auto* stepper = timeNavigation->GetStepper())
          {
            stepper->SetPos(static_cast<unsigned int>(std::lround(normalized * (m_TimeSteps - 1))));
          }
        }
      }
      break;
    case NavRow::Kind::InPlaneRight:
    case NavRow::Kind::InPlaneUp:
    {
      mitk::Point3D origin;
      mitk::Vector3D rightUnit, upUnit;
      double extentRight = 0.0, extentUp = 0.0, rightCoord = 0.0, upCoord = 0.0;
      if (!this->NavigatorInPlaneState(origin, rightUnit, upUnit, extentRight, extentUp, rightCoord, upCoord))
      {
        break;
      }
      if (row.kind == NavRow::Kind::InPlaneRight)
      {
        this->NavigatorMoveInPlane(normalized * extentRight - rightCoord, 0.0);
      }
      else
      {
        this->NavigatorMoveInPlane(0.0, normalized * extentUp - upCoord);
      }
      break;
    }
  }
}

void QmitkMxNCellOverlay::UpdateInteractivity()
{
  using State = QmitkRenderWindowProximity::State;

  // Input is only taken when a piece of interactive furniture is under the
  // pointer (colorbar drag, colormap chip, W/L click, plane-label click, or
  // the navigator sliders); everywhere else the overlay stays transparent so
  // VTK is never shadowed.
  const bool interactive = !m_CleanView
    && (m_RibbonState == State::Active || m_WindowLevelState == State::Active
        || m_PlaneLabelState == State::Active || m_BottomState == State::Active
        || m_DragMode != DragMode::None || m_NavDragRow >= 0);

  if (!interactive)
  {
    this->clearMask();
    this->setTransparentForMouseEvents(true);
    m_Hover = HoverTarget::None;
    m_HoverNavRow = -1;
    return;
  }

  // The mask clips painting as well as input, so it must cover every piece
  // of currently visible furniture, not only the interactive one.
  const QRect area = this->RenderWindowRect();
  const QRect ribbon = this->RibbonRect();
  QRegion mask;
  mask += ribbon;
  mask += this->ColormapChipRect();
  if (m_HasLevelWindow && this->IsPassiveVisible(true))
  {
    mask += this->WindowLevelRect();
  }
  if (ribbon.isValid() && m_RevealProgress > 0.0)
  {
    // The value tick scale is painted left of the widened colorbar.
    mask += QRect(ribbon.left() - TickScaleWidth, ribbon.top(), TickScaleWidth, ribbon.height());
  }

  // Paint-only passive furniture still needs mask coverage or it would vanish
  // while the intensity region is interactive.
  const QRect plane = this->PlaneLabelRect();
  if (plane.isValid())
  {
    mask += plane;
  }
  const QRect slice = this->SliceReadoutRect();
  if (slice.isValid())
  {
    mask += slice;
  }
  if (area.isValid())
  {
    mask += QRect(area.left(), area.bottom() - SliceTickHeight, area.width(), SliceTickHeight);
  }
  // The navigator band paints while the frame is revealed, so it must be in
  // the mask (which clips painting) whenever anything is revealed.
  if (m_RevealProgress > 0.0)
  {
    const QRect band = this->NavigatorBandRect();
    if (band.isValid())
    {
      mask += band;
    }
    // The expanded coordinate line takes input (hover highlight + click to edit)
    // and may sit outside the band, so add it explicitly.
    const QRect coord = this->CoordinateLineRect();
    if (coord.isValid())
    {
      mask += coord;
    }
  }
  mask += QRect(0, 0, this->width(), TopStripHeight);

  // The active-cell corner brackets sit at the frame corners; keep those small
  // squares in the mask so the brackets are not clipped while revealed.
  const int corner = 12;
  mask += QRect(area.left(), area.top(), corner, corner);
  mask += QRect(area.right() - corner + 1, area.top(), corner, corner);
  mask += QRect(area.left(), area.bottom() - corner + 1, corner, corner);
  mask += QRect(area.right() - corner + 1, area.bottom() - corner + 1, corner, corner);

  this->setMask(mask);
  this->setTransparentForMouseEvents(false);
  this->setMouseTracking(true);
}

void QmitkMxNCellOverlay::paintEvent(QPaintEvent* /*event*/)
{
  if (m_CleanView)
  {
    return;
  }

  const QRect area = this->RenderWindowRect();
  if (!area.isValid())
  {
    return;
  }

  using State = QmitkRenderWindowProximity::State;

  QPainter painter(this);
  const QFont readoutFont = ReadoutFont(this->font());

  // Active-cell mark: white corner brackets over the group-hue border (the
  // editor keeps the border in the group's color), so an active grouped cell
  // shows both its group and its focus. Always on for the active cell; not
  // gated by the reveal.
  if (nullptr != m_Editor && m_Editor->GetActiveRenderWindowWidget().get() == m_Cell)
  {
    painter.save();
    painter.setPen(QPen(QColor(0xFF, 0xFF, 0xFF), 2));
    const QRect r = area.adjusted(1, 1, -2, -2);
    const int arm = 10;
    painter.drawLine(r.left(), r.top(), r.left() + arm, r.top());
    painter.drawLine(r.left(), r.top(), r.left(), r.top() + arm);
    painter.drawLine(r.right(), r.top(), r.right() - arm, r.top());
    painter.drawLine(r.right(), r.top(), r.right(), r.top() + arm);
    painter.drawLine(r.left(), r.bottom(), r.left() + arm, r.bottom());
    painter.drawLine(r.left(), r.bottom(), r.left(), r.bottom() - arm);
    painter.drawLine(r.right(), r.bottom(), r.right() - arm, r.bottom());
    painter.drawLine(r.right(), r.bottom(), r.right(), r.bottom() - arm);
    painter.restore();
  }

  // ---- Right edge: the colorbar, widening and insetting with the reveal ----
  // At rest a thin full-height strip; on reveal it widens and pulls in top and
  // bottom so its range labels clear the top chrome and the W/L readout, and
  // the colormap chip has room below.
  const int ribbonWidth =
    PassiveRibbonWidth + qRound((ActiveRibbonWidth - PassiveRibbonWidth) * m_RevealProgress);
  const int insetTop = qRound(ColorbarInsetTop * m_RevealProgress);
  const int insetBottom = qRound(ColorbarInsetBottom * m_RevealProgress);
  const QRect ribbon(area.right() - ribbonWidth + 1, area.top() + insetTop,
                     ribbonWidth, area.height() - insetTop - insetBottom);
  if (!m_LutStrip.isNull())
  {
    painter.drawImage(ribbon, m_LutStrip);
  }
  else
  {
    painter.fillRect(ribbon, QColor(255, 255, 255, 40));
  }

  // Value tick scale left of the widened bar: labels on a few evenly-spaced
  // ticks (count scaled to the bar height), dimmed at rest and opaque under
  // the pointer like the W/L readout; the level marked in the windowing hue.
  if (m_RevealProgress > 0.0 && m_HasLevelWindow && ribbon.height() > 1)
  {
    const double upper = m_LevelWindow.GetUpperWindowBound();
    const double lower = m_LevelWindow.GetLowerWindowBound();
    const double span = upper - lower;
    if (span > 0.0)
    {
      const int slideX = qRound((1.0 - m_RevealProgress) * RevealSlideOffset);
      painter.setFont(readoutFont);
      const QFontMetrics metrics(readoutFont);
      const QColor tickColor = Faded(ActiveText, m_RevealProgress);
      const QColor labelColor =
        Faded(m_Hover == HoverTarget::Ribbon ? ActiveText : IdleText, m_RevealProgress);

      const int divisions = std::clamp(ribbon.height() / 44, 2, 6);
      for (int i = 0; i <= divisions; ++i)
      {
        const double t = static_cast<double>(i) / divisions;  // 0 = top = upper
        const int y = ribbon.top() + qRound(t * (ribbon.height() - 1));
        painter.setPen(QPen(tickColor, 1));
        painter.drawLine(ribbon.left() - 5 - slideX, y, ribbon.left() - 1 - slideX, y);

        // The end labels align to (not straddle) their tick - top label below
        // the top tick, bottom label above the bottom tick - so they stay
        // inside the bar span and are never clipped; middle labels are centred.
        const int labelRight = ribbon.left() - 7 - slideX;
        const int labelTop = (i == 0) ? y
                           : (i == divisions) ? y - metrics.height()
                           : y - metrics.height() / 2;
        painter.setPen(labelColor);
        painter.drawText(QRect(labelRight - TickScaleWidth, labelTop, TickScaleWidth, metrics.height()),
                         Qt::AlignRight | Qt::AlignVCenter, FormatValue(upper - t * span));
      }

      QColor levelColor = Faded(ActiveText, m_RevealProgress);
      const auto windowingLink =
        m_Editor->GetSyncLink(m_Cell->GetWidgetName(), QmitkMxNSyncDimension::Windowing);
      if (windowingLink.has_value())
      {
        try
        {
          levelColor = Faded(m_Editor->GetSyncGroupColor(windowingLink->group), m_RevealProgress);
        }
        catch (const mitk::Exception&)
        {
        }
      }
      const int levelY =
        ribbon.top() + qRound((upper - m_LevelWindow.GetLevel()) / span * (ribbon.height() - 1));
      painter.setPen(QPen(levelColor, 2));
      painter.drawLine(ribbon.left() - 7 - slideX, levelY, ribbon.right(), levelY);
    }
  }

  // Colormap picker chip beneath the shortened bar (reveal only).
  if (m_RevealProgress > 0.0 && m_TopNode.IsNotNull())
  {
    const QRect chip = this->ColormapChipRect();
    painter.setOpacity(m_RevealProgress);
    if (!m_LutStrip.isNull())
    {
      painter.drawImage(chip, m_LutStrip);
    }
    else
    {
      painter.fillRect(chip, QColor(255, 255, 255, 40));
    }
    painter.setPen(QPen(m_Hover == HoverTarget::Colormap ? ActiveText : IdleText, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(chip.adjusted(0, 0, -1, -1));
    painter.setOpacity(1.0);
  }

  // Drag end-handles: contrasting notches at the window bounds, shown while the
  // pointer is over the colorbar or a windowing drag is in progress.
  if ((m_Hover == HoverTarget::Ribbon || m_DragMode != DragMode::None) && m_HasLevelWindow)
  {
    painter.fillRect(QRect(ribbon.left(), ribbon.top(), ribbon.width(), 2), ActiveText);
    painter.fillRect(QRect(ribbon.left(), ribbon.bottom() - 1, ribbon.width(), 2), ActiveText);
  }

  // ---- Bottom-right: W/L readout beside the colorbar ----
  if (m_HasLevelWindow && this->IsPassiveVisible(true))
  {
    painter.setFont(readoutFont);
    painter.setPen(m_Hover == HoverTarget::WindowLevel ? ActiveText : IdleText);
    painter.drawText(this->WindowLevelRect(), Qt::AlignRight | Qt::AlignVCenter,
                     QStringLiteral("W %1 L %2").arg(FormatValue(m_LevelWindow.GetWindow()),
                                                     FormatValue(m_LevelWindow.GetLevel())));
  }

  // ---- Bottom-left: navigation (plane label, slice + dot + time) ----
  if (this->IsPassiveVisible(false))
  {
    const QString planeLabel = this->ResolvePlaneLabel();
    if (!planeLabel.isEmpty())
    {
      // Highlights opaque on hover to advertise it is clickable (reorient).
      painter.setFont(readoutFont);
      painter.setPen(m_Hover == HoverTarget::PlaneLabel ? ActiveText : IdleText);
      painter.drawText(this->PlaneLabelRect(), Qt::AlignLeft | Qt::AlignVCenter, planeLabel);
    }

    const QRect line2 = this->SliceReadoutRect();
    if (line2.isValid())
    {
      const QFontMetrics metrics(readoutFont);
      int x = line2.left();

      painter.setFont(readoutFont);
      painter.setPen(IdleText);
      if (m_SliceSteps > 0)
      {
        const QString sliceText = QStringLiteral("%1/%2").arg(m_SlicePosition + 1).arg(m_SliceSteps);
        painter.drawText(QRect(x, line2.top(), metrics.horizontalAdvance(sliceText), line2.height()),
                         Qt::AlignLeft | Qt::AlignVCenter, sliceText);
        x += metrics.horizontalAdvance(sliceText);

        // Time rides the same line, prefixed by a small clock glyph.
        if (m_TimeSteps > 1)
        {
          x += ReadoutMargin;
          const int glyph = metrics.height();
          painter.setRenderHint(QPainter::Antialiasing, true);
          DrawClockGlyph(painter, QRect(x, line2.top() + (line2.height() - glyph) / 2, glyph, glyph), IdleText);
          painter.setRenderHint(QPainter::Antialiasing, false);
          x += glyph + 2;
          const QString timeText = QStringLiteral("%1/%2").arg(m_TimePosition + 1).arg(m_TimeSteps);
          painter.setPen(IdleText);
          painter.drawText(QRect(x, line2.top(), metrics.horizontalAdvance(timeText), line2.height()),
                           Qt::AlignLeft | Qt::AlignVCenter, timeText);
        }
      }
    }
  }

  // ---- Bottom hairline: slice tick (bar) + time position (triangle) ----
  if (m_SliceSteps > 0 && this->IsPassiveVisible(false))
  {
    painter.fillRect(QRect(area.left(), area.bottom() - 1, area.width(), 1), QColor(255, 255, 255, 40));
    if (m_SliceSteps > 1)
    {
      const int tickX = area.left()
        + static_cast<int>((static_cast<double>(m_SlicePosition) / (m_SliceSteps - 1)) * (area.width() - 2));
      painter.fillRect(QRect(tickX, area.bottom() - SliceTickHeight, 2, SliceTickHeight), IdleText);
    }

    // The time-step position rides the same hairline as a distinct triangle,
    // so it costs no extra line or vertical space.
    if (m_TimeSteps > 1)
    {
      const int timeX = area.left()
        + static_cast<int>((static_cast<double>(m_TimePosition) / (m_TimeSteps - 1)) * (area.width() - 2));
      QPolygon triangle;
      triangle << QPoint(timeX - TimeTriangleHalfWidth, area.bottom() - SliceTickHeight)
               << QPoint(timeX + TimeTriangleHalfWidth, area.bottom() - SliceTickHeight)
               << QPoint(timeX, area.bottom() - SliceTickHeight + TimeTriangleHalfWidth * 2);
      painter.setRenderHint(QPainter::Antialiasing, true);
      painter.setPen(Qt::NoPen);
      painter.setBrush(ActiveText);
      painter.drawPolygon(triangle);
      painter.setRenderHint(QPainter::Antialiasing, false);
    }
  }

  // ---- Navigator: painted slider rows, revealed with the frame ----
  if (m_RevealProgress > 0.0)
  {
    const auto rows = this->NavigatorRows();
    if (!rows.empty())
    {
      // The navigator reveals faint with the frame; an individual slider row
      // brightens and thickens only while the pointer is on it (or it is being
      // dragged), signalling it is the interactive one.
      const double baseAlpha = m_RevealProgress * 0.55;

      painter.save();
      painter.translate(0, qRound((1.0 - m_RevealProgress) * RevealSlideOffset));
      const QFont labelFont = ReadoutFont(this->font());
      const QFontMetrics labelMetrics(labelFont);

      for (int i = 0; i < static_cast<int>(rows.size()); ++i)
      {
        const auto& row = rows[static_cast<std::size_t>(i)];
        const int centerY = row.track.center().y();
        const bool rowHot = (i == m_HoverNavRow) || (i == m_NavDragRow);
        const double rowAlpha = rowHot ? m_RevealProgress : baseAlpha;

        // The row label matches the other readouts' idle/hover tokens (not the
        // fainter track), fading in only with the reveal; hover lifts it to the
        // active color like the slice / plane / W-L labels.
        painter.setFont(labelFont);
        painter.setPen(Faded(rowHot ? ActiveText : IdleText, m_RevealProgress));
        painter.drawText(QRect(row.track.left() - NavLabelWidth - NavRowGap, row.track.top(),
                               NavLabelWidth, row.track.height()),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         labelMetrics.elidedText(row.label, Qt::ElideRight, NavLabelWidth));

        const int thickness = rowHot ? NavTrackThickness + 1 : NavTrackThickness;
        painter.fillRect(
          QRect(row.track.left(), centerY - thickness / 2, row.track.width(), thickness),
          Faded(IdleText, rowAlpha));

        const int knobX = row.track.left() + qRound(row.normalized * row.track.width());
        const int knobRadius = rowHot ? NavKnobRadius + 1 : NavKnobRadius;
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Faded(ActiveText, rowHot ? m_RevealProgress : baseAlpha));
        painter.drawEllipse(QPoint(knobX, centerY), knobRadius, knobRadius);
        painter.setRenderHint(QPainter::Antialiasing, false);
      }

      // Coordinate line (expanded): the crosshair world position, click to edit.
      // Faint by default; opaque only while the pointer is over it, so it reads
      // as interactive rather than permanently highlighted.
      const QRect coord = this->CoordinateLineRect();
      if (coord.isValid())
      {
        const bool coordHot = (m_Hover == HoverTarget::Coordinate);
        const mitk::Point3D world = this->CrosshairWorld();
        painter.setFont(labelFont);
        painter.setPen(Faded(coordHot ? ActiveText : IdleText, m_RevealProgress));
        painter.drawText(coord, Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("x %1  y %2  z %3 mm").arg(FormatValue(world[0]),
                                                                   FormatValue(world[1]),
                                                                   FormatValue(world[2])));
      }
      painter.restore();
    }
  }

  // ---- Top edge: collapsed utility-toolbar strip (only while the toolbar,
  //      which reveals with the whole frame, is hidden) ----
  if (!m_FrameRevealed)
  {
    painter.fillRect(QRect(0, 0, this->width(), TopStripHeight), QColor(255, 255, 255, 40));
  }
}

void QmitkMxNCellOverlay::mousePressEvent(QMouseEvent* event)
{
  const QPoint position = event->pos();

  if (event->button() == Qt::LeftButton && this->ColormapChipRect().contains(position))
  {
    event->accept();
    this->OpenColormapMenu();
    return;
  }

  // Click-to-edit readouts (the hover highlight advertises them): the W/L
  // readout opens the numeric entry, the plane label the direction picker.
  if (event->button() == Qt::LeftButton && m_HasLevelWindow && m_TopNode.IsNotNull()
      && this->WindowLevelRect().contains(position))
  {
    event->accept();
    this->OpenNumericEntry();
    return;
  }
  if (event->button() == Qt::LeftButton && !this->ResolvePlaneLabel().isEmpty()
      && this->PlaneLabelRect().contains(position))
  {
    event->accept();
    this->OpenDirectionPicker(event->globalPosition().toPoint());
    return;
  }

  const QRect ribbon = this->RibbonRect();
  if (event->button() == Qt::LeftButton && ribbon.contains(position)
      && m_HasLevelWindow && m_TopNode.IsNotNull())
  {
    if (position.y() < ribbon.top() + RibbonEndHandleHeight)
    {
      m_DragMode = DragMode::UpperBound;
    }
    else if (position.y() > ribbon.bottom() - RibbonEndHandleHeight)
    {
      m_DragMode = DragMode::LowerBound;
    }
    else
    {
      m_DragMode = DragMode::Level;
    }
    m_LastDragPosition = position;
    m_DragScale = this->DragScale();
    event->accept();
    return;
  }

  // Navigator: the coordinate line opens the click-to-edit entry; a slider row
  // starts a drag and jumps to the clicked position.
  if (event->button() == Qt::LeftButton)
  {
    if (this->CoordinateLineRect().contains(position))
    {
      event->accept();
      this->OpenCoordinateEntry();
      return;
    }
    const auto rows = this->NavigatorRows();
    for (std::size_t i = 0; i < rows.size(); ++i)
    {
      if (rows[i].track.contains(position))
      {
        m_NavDragRow = static_cast<int>(i);
        this->ApplyNavigatorRow(rows[i],
          static_cast<double>(position.x() - rows[i].track.left()) / std::max(1, rows[i].track.width()));
        event->accept();
        return;
      }
    }
  }

  event->ignore();
}

QmitkMxNCellOverlay::HoverTarget QmitkMxNCellOverlay::HoverAt(const QPoint& pos) const
{
  // Specific / small targets first; the colorbar is the largest and last, so a
  // chip or readout sitting near it wins the hit test.
  if (this->ColormapChipRect().contains(pos))
  {
    return HoverTarget::Colormap;
  }
  if (m_HasLevelWindow && this->WindowLevelRect().contains(pos))
  {
    return HoverTarget::WindowLevel;
  }
  if (this->PlaneLabelRect().contains(pos))
  {
    return HoverTarget::PlaneLabel;
  }
  if (this->CoordinateLineRect().contains(pos))
  {
    return HoverTarget::Coordinate;
  }
  if (this->RibbonRect().contains(pos))
  {
    return HoverTarget::Ribbon;
  }
  return HoverTarget::None;
}

void QmitkMxNCellOverlay::leaveEvent(QEvent* event)
{
  if (m_Hover != HoverTarget::None || m_HoverNavRow != -1)
  {
    m_Hover = HoverTarget::None;
    m_HoverNavRow = -1;
    this->update();
  }
  QmitkOverlayWidget::leaveEvent(event);
}

void QmitkMxNCellOverlay::mouseMoveEvent(QMouseEvent* event)
{
  if (m_NavDragRow >= 0)
  {
    const auto rows = this->NavigatorRows();
    if (m_NavDragRow < static_cast<int>(rows.size()))
    {
      const auto& row = rows[static_cast<std::size_t>(m_NavDragRow)];
      this->ApplyNavigatorRow(row,
        static_cast<double>(event->pos().x() - row.track.left()) / std::max(1, row.track.width()));
      this->RenderCellNow();  // present the change live during the drag
    }
    event->accept();
    return;
  }

  if (m_DragMode == DragMode::None)
  {
    // Highlight only what the pointer is actually over (not merely near),
    // decoupling per-element highlight from the proximity-driven reveal - for
    // the intensity/plane readouts (m_Hover) and the navigator slider rows.
    const HoverTarget hover = this->HoverAt(event->pos());
    int navRow = -1;
    const auto rows = this->NavigatorRows();
    for (int i = 0; i < static_cast<int>(rows.size()); ++i)
    {
      if (rows[static_cast<std::size_t>(i)].track.contains(event->pos()))
      {
        navRow = i;
        break;
      }
    }
    if (hover != m_Hover || navRow != m_HoverNavRow)
    {
      m_Hover = hover;
      m_HoverNavRow = navRow;
      this->update();
    }

    if (this->RibbonRect().contains(event->pos()))
    {
      this->setCursor(Qt::SizeVerCursor);
    }
    else if (m_Hover == HoverTarget::Coordinate || navRow >= 0)
    {
      this->setCursor(Qt::PointingHandCursor);
    }
    else
    {
      this->setCursor(Qt::ArrowCursor);
    }
    event->ignore();
    return;
  }

  const int dy = event->pos().y() - m_LastDragPosition.y();
  m_LastDragPosition = event->pos();
  if (dy == 0 || m_TopNode.IsNull())
  {
    event->accept();
    return;
  }

  // Screen-up increases the value; the scale is pinned at drag start so the
  // gesture stays linear while the window changes under it.
  const double valueDelta = -dy * m_DragScale;
  double levelDelta = 0.0;
  double windowDelta = 0.0;

  switch (m_DragMode)
  {
    case DragMode::Level:
      levelDelta = valueDelta;
      break;
    case DragMode::UpperBound:
      levelDelta = valueDelta / 2.0;
      windowDelta = valueDelta;
      break;
    case DragMode::LowerBound:
      levelDelta = valueDelta / 2.0;
      windowDelta = -valueDelta;
      break;
    default:
      break;
  }

  // Keep the window positive; collapsing it to zero would flip the bounds.
  if (m_HasLevelWindow && m_LevelWindow.GetWindow() + windowDelta < 1.0)
  {
    event->accept();
    return;
  }

  try
  {
    m_Editor->AdjustLevelWindow(m_Cell->GetWidgetName(), m_TopNode, levelDelta, windowDelta);
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Level/window drag ignored: " << e.GetDescription();
    m_DragMode = DragMode::None;
  }

  // Update the local cache eagerly so consecutive drag steps clamp against
  // the value just written; the render-end refresh re-syncs authoritatively.
  if (m_HasLevelWindow)
  {
    m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel() + levelDelta,
                                 m_LevelWindow.GetWindow() + windowDelta);
    this->update();
  }
  this->RenderCellNow();  // present the windowing change live during the drag

  event->accept();
}

void QmitkMxNCellOverlay::mouseReleaseEvent(QMouseEvent* event)
{
  if (m_NavDragRow >= 0)
  {
    m_NavDragRow = -1;
    this->UpdateInteractivity();
    event->accept();
    return;
  }

  if (m_DragMode != DragMode::None)
  {
    m_DragMode = DragMode::None;
    this->UpdateInteractivity();
    event->accept();
    return;
  }

  event->ignore();
}

void QmitkMxNCellOverlay::OpenNumericEntry()
{
  auto* popup = new QFrame(this, Qt::Popup);
  popup->setAttribute(Qt::WA_DeleteOnClose);
  popup->setFrameShape(QFrame::StyledPanel);

  auto* layout = new QFormLayout(popup);
  layout->setContentsMargins(6, 6, 6, 6);

  auto* levelBox = new QDoubleSpinBox(popup);
  levelBox->setRange(-1.0e9, 1.0e9);
  levelBox->setDecimals(1);
  levelBox->setValue(m_LevelWindow.GetLevel());

  auto* windowBox = new QDoubleSpinBox(popup);
  windowBox->setRange(1.0e-3, 1.0e9);
  windowBox->setDecimals(1);
  windowBox->setValue(m_LevelWindow.GetWindow());

  layout->addRow(tr("Level"), levelBox);
  layout->addRow(tr("Window"), windowBox);

  const auto commit = [this, levelBox, windowBox]()
  {
    if (m_TopNode.IsNull())
    {
      return;
    }
    try
    {
      m_Editor->SetLevelWindow(m_Cell->GetWidgetName(), m_TopNode,
                               mitk::LevelWindow(levelBox->value(), windowBox->value()));
    }
    catch (const mitk::Exception& e)
    {
      MITK_WARN << "Numeric level/window entry ignored: " << e.GetDescription();
    }
  };
  connect(levelBox, &QDoubleSpinBox::editingFinished, this, commit);
  connect(windowBox, &QDoubleSpinBox::editingFinished, this, commit);

  const QRect readout = this->WindowLevelRect();
  popup->adjustSize();
  popup->move(this->mapToGlobal(QPoint(readout.left(), readout.top() - popup->sizeHint().height() - 4)));
  popup->show();
  levelBox->setFocus();
  levelBox->selectAll();
}

void QmitkMxNCellOverlay::OpenCoordinateEntry()
{
  const mitk::Point3D world = this->CrosshairWorld();
  const bool hasIndex = m_TopNode.IsNotNull() && nullptr != m_TopNode->GetData()
    && nullptr != m_TopNode->GetData()->GetGeometry();
  mitk::Point3D index;
  if (hasIndex)
  {
    m_TopNode->GetData()->GetGeometry()->WorldToIndex(world, index);
  }

  auto* popup = new QFrame(this, Qt::Popup);
  popup->setAttribute(Qt::WA_DeleteOnClose);
  popup->setFrameShape(QFrame::StyledPanel);

  auto* layout = new QFormLayout(popup);
  layout->setContentsMargins(6, 6, 6, 6);

  auto* worldRow = new QWidget(popup);
  auto* worldLayout = new QHBoxLayout(worldRow);
  worldLayout->setContentsMargins({});
  std::array<QDoubleSpinBox*, 3> worldBoxes{};
  for (int axis = 0; axis < 3; ++axis)
  {
    auto* box = new QDoubleSpinBox(worldRow);
    box->setRange(-1.0e6, 1.0e6);
    box->setDecimals(2);
    box->setValue(world[axis]);
    worldLayout->addWidget(box);
    worldBoxes[static_cast<std::size_t>(axis)] = box;
  }
  layout->addRow(tr("World (mm)"), worldRow);

  const auto commitWorld = [this, worldBoxes]()
  {
    mitk::Point3D position;
    for (int axis = 0; axis < 3; ++axis)
    {
      position[axis] = worldBoxes[static_cast<std::size_t>(axis)]->value();
    }
    this->NavigatorSetCrosshair(position);
  };
  for (auto* box : worldBoxes)
  {
    connect(box, &QDoubleSpinBox::editingFinished, this, commitWorld);
  }

  if (hasIndex)
  {
    auto* indexRow = new QWidget(popup);
    auto* indexLayout = new QHBoxLayout(indexRow);
    indexLayout->setContentsMargins({});
    std::array<QSpinBox*, 3> indexBoxes{};
    for (int axis = 0; axis < 3; ++axis)
    {
      auto* box = new QSpinBox(indexRow);
      box->setRange(-100000, 100000);
      box->setValue(static_cast<int>(std::lround(index[axis])));
      indexLayout->addWidget(box);
      indexBoxes[static_cast<std::size_t>(axis)] = box;
    }
    layout->addRow(tr("Voxel index"), indexRow);

    const auto commitIndex = [this, indexBoxes]()
    {
      mitk::Point3D voxel;
      for (int axis = 0; axis < 3; ++axis)
      {
        voxel[axis] = indexBoxes[static_cast<std::size_t>(axis)]->value();
      }
      this->NavigatorSetVoxelIndex(voxel);
    };
    for (auto* box : indexBoxes)
    {
      connect(box, &QSpinBox::editingFinished, this, commitIndex);
    }
  }

  const QRect coord = this->CoordinateLineRect();
  popup->adjustSize();
  popup->move(this->mapToGlobal(QPoint(coord.left(), coord.top() - popup->sizeHint().height() - 4)));
  popup->show();
  worldBoxes[0]->setFocus();
  worldBoxes[0]->selectAll();
}

void QmitkMxNCellOverlay::OpenColormapMenu()
{
  if (m_TopNode.IsNull())
  {
    return;
  }

  QMenu menu(this);
  const auto& names = mitk::LookupTable::typenameList;
  for (std::size_t i = 0; i < names.size(); ++i)
  {
    auto* action = menu.addAction(QString::fromStdString(names[i]));
    action->setData(static_cast<int>(i));
  }

  auto* chosen = menu.exec(this->mapToGlobal(this->ColormapChipRect().bottomLeft()));
  if (nullptr == chosen)
  {
    return;
  }

  auto lookupTable = mitk::LookupTable::New();
  lookupTable->SetType(static_cast<mitk::LookupTable::LookupTableType>(chosen->data().toInt()));

  try
  {
    m_Editor->SetLookupTable(m_Cell->GetWidgetName(), m_TopNode, lookupTable);
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Colormap selection ignored: " << e.GetDescription();
  }
}

void QmitkMxNCellOverlay::OpenDirectionPicker(const QPoint& globalPosition)
{
  const auto windowId = m_Cell->GetWidgetName();

  QMenu menu(this);
  const std::pair<const char*, mitk::AnatomicalPlane> planes[] = {
    { "Axial", mitk::AnatomicalPlane::Axial },
    { "Coronal", mitk::AnatomicalPlane::Coronal },
    { "Sagittal", mitk::AnatomicalPlane::Sagittal },
  };
  for (const auto& [label, plane] : planes)
  {
    auto* action = menu.addAction(tr(label));
    const auto planeValue = plane;
    connect(action, &QAction::triggered, this, [this, windowId, planeValue]()
    {
      try
      {
        m_Editor->SetViewDirection(windowId, planeValue);
      }
      catch (const mitk::Exception& e)
      {
        MITK_WARN << "View direction ignored: " << e.GetDescription();
      }
    });
  }
  menu.exec(globalPosition);
}

double QmitkMxNCellOverlay::DragScale() const
{
  const int height = std::max(1, this->RenderWindowRect().height());
  const double window = m_HasLevelWindow ? std::max(1.0, m_LevelWindow.GetWindow()) : 256.0;
  return window / height;
}

void QmitkMxNCellOverlay::RenderCellNow()
{
  // Regenerate (camera prep + VTK render) through the rendering manager, then
  // synchronously repaint the widget. The VTK render alone only queues a
  // paintGL on the QVTKOpenGLNativeWidget, and that queued paint is starved by
  // the continuous mouse-move stream of a drag; repaint() presents it now.
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(m_VtkRenderWindow);
  if (auto* renderWindow = m_Cell->GetRenderWindow())
  {
    renderWindow->repaint();
  }
}

void QmitkMxNCellOverlay::OnVtkRenderEnd(vtkObject* /*caller*/, unsigned long /*eventId*/,
                                         void* clientData, void* /*callData*/)
{
  static_cast<QmitkMxNCellOverlay*>(clientData)->ScheduleValueRefresh();
}

void QmitkMxNCellOverlay::ScheduleValueRefresh()
{
  if (m_RefreshPending)
  {
    return;
  }

  m_RefreshPending = true;
  QTimer::singleShot(0, this, [this]()
  {
    m_RefreshPending = false;
    this->RefreshValues();
  });
}

void QmitkMxNCellOverlay::RefreshValues()
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer)
  {
    return;
  }

  const auto node = this->ResolveTopImageNode();

  mitk::LevelWindow levelWindow;
  const bool hasLevelWindow = node.IsNotNull() && node->GetLevelWindow(levelWindow, renderer);

  vtkMTimeType lutMTime = 0;
  if (node.IsNotNull())
  {
    if (auto* lutProperty = dynamic_cast<mitk::LookupTableProperty*>(
          node->GetProperty("LookupTable", renderer)))
    {
      if (lutProperty->GetLookupTable() != nullptr
          && lutProperty->GetLookupTable()->GetVtkLookupTable() != nullptr)
      {
        lutMTime = lutProperty->GetLookupTable()->GetVtkLookupTable()->GetMTime();
      }
    }
  }

  unsigned int slicePosition = 0;
  unsigned int sliceSteps = 0;
  if (auto* sliceStepper = renderer->GetSliceNavigationController()->GetStepper())
  {
    slicePosition = sliceStepper->GetPos();
    sliceSteps = sliceStepper->GetSteps();
  }
  unsigned int timePosition = 0;
  unsigned int timeSteps = 0;
  if (auto* timeNavigation = mitk::RenderingManager::GetInstance()->GetTimeNavigationController())
  {
    if (auto* timeStepper = timeNavigation->GetStepper())
    {
      timePosition = timeStepper->GetPos();
      timeSteps = timeStepper->GetSteps();
    }
  }

  const bool changed = node != m_TopNode || hasLevelWindow != m_HasLevelWindow
    || lutMTime != m_LutMTime
    || slicePosition != m_SlicePosition || sliceSteps != m_SliceSteps
    || timePosition != m_TimePosition || timeSteps != m_TimeSteps
    || (hasLevelWindow
        && (levelWindow.GetLevel() != m_LevelWindow.GetLevel()
            || levelWindow.GetWindow() != m_LevelWindow.GetWindow()));

  if (!changed)
  {
    return;
  }

  m_TopNode = node;
  m_HasLevelWindow = hasLevelWindow;
  if (hasLevelWindow)
  {
    m_LevelWindow = levelWindow;
  }
  m_LutMTime = lutMTime;
  m_SlicePosition = slicePosition;
  m_SliceSteps = sliceSteps;
  m_TimePosition = timePosition;
  m_TimeSteps = timeSteps;

  this->RebuildLutStrip();
  this->UpdateInteractivity();
  this->update();
}

mitk::DataNode::Pointer QmitkMxNCellOverlay::ResolveTopImageNode() const
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer || nullptr == renderer->GetDataStorage())
  {
    return nullptr;
  }

  const auto imageNodes = renderer->GetDataStorage()->GetSubset(
    mitk::NodePredicateDataType::New("Image"));

  mitk::DataNode::Pointer topNode;
  int topLayer = std::numeric_limits<int>::min();
  for (const auto& node : *imageNodes)
  {
    if (node.IsNull() || !node->IsVisible(renderer))
    {
      continue;
    }
    int layer = 0;
    node->GetIntProperty("layer", layer, renderer);
    if (topNode.IsNull() || layer > topLayer)
    {
      topNode = node;
      topLayer = layer;
    }
  }

  return topNode;
}

void QmitkMxNCellOverlay::RebuildLutStrip()
{
  m_LutStrip = QImage();

  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (m_TopNode.IsNull() || nullptr == renderer)
  {
    return;
  }

  auto* lutProperty = dynamic_cast<mitk::LookupTableProperty*>(
    m_TopNode->GetProperty("LookupTable", renderer));
  vtkLookupTable* lookupTable = nullptr;
  if (nullptr != lutProperty && lutProperty->GetLookupTable() != nullptr)
  {
    lookupTable = lutProperty->GetLookupTable()->GetVtkLookupTable();
  }

  constexpr int Samples = 256;
  QImage strip(1, Samples, QImage::Format_RGB32);
  for (int i = 0; i < Samples; ++i)
  {
    // Top row = highest value, matching the ribbon's top = upper bound.
    const double t = 1.0 - static_cast<double>(i) / (Samples - 1);
    QColor color;
    if (nullptr != lookupTable)
    {
      const double* range = lookupTable->GetRange();
      double rgb[3] = { 0.0, 0.0, 0.0 };
      lookupTable->GetColor(range[0] + t * (range[1] - range[0]), rgb);
      color = QColor::fromRgbF(rgb[0], rgb[1], rgb[2]);
    }
    else
    {
      // No LUT property: the mapper renders grayscale through level/window.
      color = QColor::fromRgbF(t, t, t);
    }
    strip.setPixel(0, i, color.rgb());
  }

  m_LutStrip = strip;
}

bool QmitkMxNCellOverlay::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == m_Cell->GetRenderWindow())
  {
    if (event->type() == QEvent::MouseButtonPress
        && static_cast<QMouseEvent*>(event)->button() == Qt::RightButton)
    {
      m_RightPressPosition = static_cast<QMouseEvent*>(event)->pos();
    }
    else if (event->type() == QEvent::ContextMenu)
    {
      // The right button doubles as the zoom / windowing gesture; only a
      // click without drag is a context-menu request. Consuming the event in
      // both cases keeps a gesture's release from popping the menu. The menu
      // stays available in clean-view: it is transient (right-click only, not
      // passive furniture) and it is the way back - it carries the clean-view
      // toggle, which is otherwise unreachable once the utility strip hides.
      auto* contextEvent = static_cast<QContextMenuEvent*>(event);
      if ((contextEvent->pos() - m_RightPressPosition).manhattanLength()
            < QApplication::startDragDistance())
      {
        this->OpenContextMenu(contextEvent->globalPos());
      }
      return true;
    }
  }

  return QmitkOverlayWidget::eventFilter(watched, event);
}

void QmitkMxNCellOverlay::OpenContextMenu(const QPoint& globalPosition)
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer)
  {
    return;
  }
  const auto windowId = m_Cell->GetWidgetName();

  QMenu menu(this);

  // Per-renderer data visibility: toggles affect only this cell.
  if (auto dataStorage = renderer->GetDataStorage(); dataStorage.IsNotNull())
  {
    const auto noHelperObjects = mitk::NodePredicateAnd::New();
    noHelperObjects->AddPredicate(
      mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
    noHelperObjects->AddPredicate(
      mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

    auto* visibilityMenu = menu.addMenu(tr("Shown data"));
    for (const auto& node : *dataStorage->GetSubset(noHelperObjects))
    {
      if (node.IsNull())
      {
        continue;
      }
      auto* action = visibilityMenu->addAction(QString::fromStdString(node->GetName()));
      action->setCheckable(true);
      action->setChecked(node->IsVisible(renderer));
      mitk::DataNode::Pointer heldNode = node;
      connect(action, &QAction::toggled, this, [this, heldNode](bool visible)
      {
        auto* localRenderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
        if (nullptr != localRenderer)
        {
          heldNode->SetVisibility(visible, localRenderer);
          mitk::RenderingManager::GetInstance()->RequestUpdate(m_VtkRenderWindow);
        }
      });
    }
    visibilityMenu->setEnabled(!visibilityMenu->isEmpty());
  }

  auto* directionMenu = menu.addMenu(tr("View direction"));
  const std::pair<const char*, mitk::AnatomicalPlane> planes[] = {
    { "Axial", mitk::AnatomicalPlane::Axial },
    { "Coronal", mitk::AnatomicalPlane::Coronal },
    { "Sagittal", mitk::AnatomicalPlane::Sagittal },
  };
  for (const auto& [label, plane] : planes)
  {
    auto* action = directionMenu->addAction(tr(label));
    const auto planeValue = plane;
    connect(action, &QAction::triggered, this, [this, windowId, planeValue]()
    {
      try
      {
        m_Editor->SetViewDirection(windowId, planeValue);
      }
      catch (const mitk::Exception& e)
      {
        MITK_WARN << "Context menu: view direction ignored: " << e.GetDescription();
      }
    });
  }

  menu.addSeparator();

  auto* reinitAction = menu.addAction(tr("Reinit group geometry"));
  connect(reinitAction, &QAction::triggered, this, [this, windowId]()
  {
    try
    {
      m_Editor->ReinitSyncGroupGeometry(windowId);
    }
    catch (const mitk::Exception& e)
    {
      MITK_WARN << "Context menu: geometry reinit ignored: " << e.GetDescription();
    }
  });

  auto* reconvergeAction = menu.addAction(tr("Re-converge sync groups"));
  bool hasOffsetLink = false;
  for (const auto dimension : { QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Zoom,
                                QmitkMxNSyncDimension::Pan })
  {
    hasOffsetLink = hasOffsetLink || m_Editor->GetSyncLink(windowId, dimension).has_value();
  }
  reconvergeAction->setEnabled(hasOffsetLink);
  connect(reconvergeAction, &QAction::triggered, this, [this, windowId]()
  {
    for (const auto dimension : { QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Zoom,
                                  QmitkMxNSyncDimension::Pan })
    {
      const auto link = m_Editor->GetSyncLink(windowId, dimension);
      if (!link.has_value())
      {
        continue;
      }
      try
      {
        m_Editor->ReconvergeSyncGroup(dimension, link->group);
      }
      catch (const mitk::Exception& e)
      {
        MITK_WARN << "Context menu: re-converge ignored: " << e.GetDescription();
      }
    }
  });

  menu.addSeparator();

  auto* editorAction = menu.addAction(tr("Open layout editor"));
  connect(editorAction, &QAction::triggered, this, [this]()
  {
    m_Editor->RequestLayoutEditor();
  });

  auto* expandedNavigatorAction = menu.addAction(tr("Expanded navigator"));
  expandedNavigatorAction->setCheckable(true);
  expandedNavigatorAction->setChecked(m_Editor->IsNavigatorExpanded());
  connect(expandedNavigatorAction, &QAction::toggled, this, [this](bool expanded)
  {
    m_Editor->SetNavigatorExpanded(expanded);
  });

  auto* cleanViewAction = menu.addAction(tr("Clean view"));
  cleanViewAction->setCheckable(true);
  cleanViewAction->setChecked(m_Editor->IsCleanView());
  connect(cleanViewAction, &QAction::toggled, this, [this](bool cleanView)
  {
    m_Editor->SetCleanView(cleanView);
  });

  menu.exec(globalPosition);
}
