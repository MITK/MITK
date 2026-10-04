/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNCellOverlay.h"

#include <QmitkMxNArrangeMode.h>
#include <QmitkMxNAxisGlyph.h>
#include <QmitkIconTheme.h>
#include <QmitkMxNGroupJoinMode.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowUtilityWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkAnatomicalPlanes.h>
#include <mitkBaseRenderer.h>
#include <mitkDisplayActionEvents.h>
#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <mitkInteractionEvent.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkPlaneGeometry.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>
#include <mitkSliceNavigationHelper.h>
#include <mitkTimeNavigationController.h>

#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkLookupTable.h>
#include <vtkRenderWindow.h>

#include <QApplication>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDoubleSpinBox>
#include <QFontMetrics>
#include <QFormLayout>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPointer>
#include <QPolygon>
#include <QPropertyAnimation>
#include <QScreen>
#include <QSpinBox>
#include <QStyle>
#include <QTimer>
#include <QToolTip>
#include <QWheelEvent>

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
  constexpr double NavLabelMaxFraction = 0.4;  // cap on the label column's share of a row
  constexpr int NavKnobRadius = 5;
  constexpr int NavKnobHotRadius = NavKnobRadius + 1;  // the hovered or dragged knob
  constexpr int NavLabelGap = 6;          // between a row's label and its knob travel
  constexpr int NavTrackThickness = 2;
  // The active colorbar does not span the whole edge: it is inset top and
  // bottom so the range labels never crowd the top chrome or the W/L readout,
  // and so the colormap chip has room below it.
  constexpr int ColorbarInsetTop = 30;
  constexpr int ColorbarInsetBottom = 46;

  const QColor IdleText(255, 255, 255, 140);    // 55 % white
  const QColor ActiveText(255, 255, 255, 216);  // 85 % white

  // ---- Sync peek plate ----
  constexpr int PeekGlyphGap = 8;         // between two glyph boxes in the row
  constexpr int PeekPlatePadX = 14;
  constexpr int PeekPlatePadY = 12;
  constexpr int PeekCaptionGap = 6;       // caption baseline block to glyph row
  constexpr int PeekValueGap = 1;         // a glyph to the ink of the value under it
  constexpr int PeekNameGap = 7;          // glyph row to the window name
  constexpr int PeekPlateRadius = 7;
  constexpr double PeekPumpScale = 1.75;  // the pointed-at glyph's side, in boxes
  // The plate is content-sized, never a scrim: it may claim this much of the
  // cell and no more, which is also what decides whether a cell hosts one.
  constexpr double PeekPlateWidthFraction = 0.86;
  constexpr double PeekPlateHeightFraction = 0.60;
  constexpr int PeekFadeInMs = 120;
  constexpr int PeekFadeOutMs = 80;
  constexpr double PeekAbsentWeight = 0.35;  // an axis this window does not synchronize

  const QColor PeekPlateFill(7, 9, 11, 158);
  const QColor PeekPlateBorder(255, 255, 255, 26);
  // The recolor path takes an RGB hex and drops any alpha, so an unsynchronized
  // axis has to read as absent by hue: a flat neutral no group palette entry can
  // be mistaken for.
  const QColor PeekAbsentGlyph(144, 150, 156);

  // ---- Arrange mode ----
  constexpr int PlateButtonSize = 16;
  constexpr int PlateButtonInset = 5;   // from the plate's top and right edge
  constexpr int PlateButtonGap = 2;     // between the menu and the close button
  const QColor PlateButtonHover(255, 255, 255, 56);
  // Twice the cell frame's stylesheet border, painted over it: the frame that
  // already names a cell's group is the one that answers "who shares this".
  constexpr int ArrangeFrameWidth = 4;

  int PeekPumpedSide(int box)
  {
    return qRound(PeekPumpScale * box);
  }

  /** \brief How far the pumped glyph reaches past its slot on the side where it
   *         reaches further: centring an odd growth leaves the two sides a
   *         pixel apart, and the row has to cover the larger one. */
  int PeekPumpOverhang(int box)
  {
    return (PeekPumpedSide(box) - box + 1) / 2;
  }

  /** \brief A glyph row's width: its slots plus the pumped glyph's overhang
   *         at either end, so the row is the same width whichever axis is
   *         pumped. */
  int PeekRowWidth(int box, QmitkMxNPeekRows rows)
  {
    const int columns = QmitkMxNPeekColumns(rows);
    return columns * box + (columns - 1) * PeekGlyphGap + 2 * PeekPumpOverhang(box);
  }

  int PeekRowHeight(int box)
  {
    return PeekPumpedSide(box);
  }

  /** \brief A pumped offset value's line height: the value grows with the glyph
   *         it belongs to, so it is legible exactly where the pointer is. */
  int PeekValueLineHeight(int textLineHeight)
  {
    return qRound(PeekPumpScale * textLineHeight);
  }

  /** \brief Where a plate's parts sit vertically, measured from its top. */
  struct PeekPlateVertical
  {
    int captionTop;
    int firstRowTop;   // the top of the first row's band, which fits a pumped glyph
    int secondRowTop;  // likewise for the second row; equals firstRowTop for one row
    int valuesTop;
    int valuesBottom;
    int nameTop;
    int height;
  };

  /**
   * \brief The plate's vertical layout, shared by its size and its parts so the
   *        two cannot drift. Every text line is reserved even when the window
   *        has no name or no offset, so every plate of a layout is one object
   *        repeated rather than eight differently sized ones.
   *
   * One row reserves the value line at its pumped height under the row, so
   * nothing overlaps. Two rows are packed instead: between them sits only a
   * resting value line and a gap that scales with the box, and the pumped
   * glyph's overhang and its grown value may reach into the other row - they
   * are painted in front. Reserving the pumped extent there would put a gap of
   * more than two glyphs between small rows. Only pan, zoom and slice carry
   * offsets, and they all sit in the first row, so the second needs no value
   * line.
   */
  PeekPlateVertical PeekPlateVerticalLayout(int box, int textLineHeight, QmitkMxNPeekRows rows)
  {
    const int rowHeight = PeekRowHeight(box);
    PeekPlateVertical v;
    v.captionTop = PeekPlatePadY;
    v.firstRowTop = v.captionTop + textLineHeight + PeekCaptionGap;
    v.valuesTop = v.firstRowTop + (rowHeight + box) / 2;
    int rowsBottom = 0;
    if (QmitkMxNPeekRows::One == rows)
    {
      v.valuesBottom = v.firstRowTop + rowHeight + PeekValueGap + PeekValueLineHeight(textLineHeight);
      v.secondRowTop = v.firstRowTop;
      rowsBottom = v.valuesBottom;
    }
    else
    {
      v.valuesBottom = v.valuesTop + PeekValueGap + textLineHeight;
      const int secondRestingTop = v.valuesBottom + std::max(2, box / 5);
      v.secondRowTop = secondRestingTop - (rowHeight - box) / 2;
      rowsBottom = v.secondRowTop + rowHeight;
    }
    v.nameTop = rowsBottom + PeekNameGap;
    v.height = v.nameTop + textLineHeight + PeekPlatePadY;
    return v;
  }

  QSize PeekPlateSize(int box, int textLineHeight, QmitkMxNPeekRows rows)
  {
    return QSize(PeekRowWidth(box, rows) + 2 * PeekPlatePadX,
                 PeekPlateVerticalLayout(box, textLineHeight, rows).height);
  }

  /** \brief A navigator row's grab area: its track plus the knob's reach at
   *         both ends, so the knob stays grabbable where it sits at an extreme. */
  QRect NavGrabRect(const QRect& track)
  {
    return track.adjusted(-NavKnobHotRadius, 0, NavKnobHotRadius, 0);
  }

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

  /** Holds the cell's furniture revealed while a modal popup opened from it
   *  runs: the popup's grab reads to the cell as the pointer leaving. */
  class ScopedProximityPin
  {
  public:
    explicit ScopedProximityPin(QmitkRenderWindowProximity* proximity)
      : m_Proximity(proximity)
    {
      if (!m_Proximity.isNull())
      {
        m_Proximity->SetPinned(true);
      }
    }

    ~ScopedProximityPin()
    {
      // The popup's event loop can destroy the cell, and the controller with it.
      if (!m_Proximity.isNull())
      {
        m_Proximity->SetPinned(false);
      }
    }

    ScopedProximityPin(const ScopedProximityPin&) = delete;
    ScopedProximityPin& operator=(const ScopedProximityPin&) = delete;

  private:
    QPointer<QmitkRenderWindowProximity> m_Proximity;
  };
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

  // The colorbar's level-marker hue is resolved live from the group state at
  // paint time, but a grouping change in the layout editor need not trigger a
  // render - so repaint on the editor's link-change signal to keep it current.
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

  // The sync peek fades on its own clock: it answers a deliberate pointer rest
  // rather than the frame's approach, and it leaves faster than it arrives.
  m_PeekAnimation = new QPropertyAnimation(this, "peekProgress", this);

  // Context-menu handling needs the render window's own mouse stream (the
  // overlay is transparent there); the base class already filters the cell.
  cell->GetRenderWindow()->installEventFilter(this);

  // Arrange mode is read here as well as followed: a cell created while it is
  // on must take group drops from the start. The drops are taken on the cell,
  // which is an ancestor of both the render window and this overlay, so a drop
  // arrives whichever of the two is under the pointer - and the render window's
  // own data-node drop handling stays switched off.
  auto* arrangeMode = m_Editor->GetArrangeMode();
  cell->setAcceptDrops(arrangeMode->IsActive());
  connect(arrangeMode, &QmitkMxNArrangeMode::ActiveChanged, this, [this](bool active)
  {
    m_Cell->setAcceptDrops(active);
    if (!active)
    {
      m_PlatePressActive = false;
      m_PlateDragArmed = false;
      m_PlateHovered = false;
      m_PlateHoverAxis = -1;
      m_DropTarget = false;
    }
    this->UpdateInteractivity();
    this->update();
  });
  connect(arrangeMode, &QmitkMxNArrangeMode::SelectionChanged, this, [this]() { this->update(); });
  connect(arrangeMode, &QmitkMxNArrangeMode::HighlightChanged, this, [this]()
  {
    this->UpdateInteractivity();
    this->update();
  });

  // Keep a reference of our own: the observer must be removable in the
  // destructor regardless of the Qt child destruction order.
  m_VtkRenderWindow = cell->GetRenderWindow()->GetVtkRenderWindow();

  auto callback = vtkSmartPointer<vtkCallbackCommand>::New();
  callback->SetClientData(this);
  callback->SetCallback(&QmitkMxNCellOverlay::OnVtkRenderEnd);
  m_VtkObserverTag = m_VtkRenderWindow->AddObserver(vtkCommand::EndEvent, callback);

  // The controller applies a region's initial state without emitting it.
  m_RibbonState = proximity->GetRegionState(m_RibbonRegion);
  m_WindowLevelState = proximity->GetRegionState(m_WindowLevelRegion);
  m_PlaneLabelState = proximity->GetRegionState(m_PlaneLabelRegion);
  m_BottomState = proximity->GetRegionState(m_BottomRegion);
  m_TopState = proximity->GetRegionState(m_TopRegion);
  this->UpdateReveal();
  this->UpdateInteractivity();

  this->ScheduleValueRefresh();
}

QmitkMxNCellOverlay::~QmitkMxNCellOverlay()
{
  if (nullptr != m_VtkRenderWindow)
  {
    m_VtkRenderWindow->RemoveObserver(m_VtkObserverTag);
  }

  // The region callbacks capture this overlay; a controller that outlives it
  // must not call them again.
  if (!m_Proximity.isNull())
  {
    for (const auto region : { m_RibbonRegion, m_WindowLevelRegion, m_PlaneLabelRegion, m_BottomRegion, m_TopRegion })
    {
      if (region >= 0)
      {
        m_Proximity->UnregisterRegion(region);
      }
    }
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
  // The proximity may already have settled while clean view still held the
  // frame down, and then emits nothing more for the pointer resting where it is.
  this->UpdateReveal();
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

int QmitkMxNCellOverlay::PeekTextLineHeight(const QFont& baseFont)
{
  return QFontMetrics(ReadoutFont(baseFont)).height();
}

int QmitkMxNCellOverlay::MaxPeekGlyphBox(const QSize& cellSize, int textLineHeight, QmitkMxNPeekRows rows)
{
  // Both plate dimensions grow monotonically with the box, so walking up from
  // the legible floor and stopping at the first box that overflows gives the
  // largest that fits. A row is wider than its boxes, which bounds the walk.
  const int ceiling = cellSize.width() / QmitkMxNPeekColumns(rows) + 1;
  int best = 0;
  for (int box = PeekGlyphBoxMin; box <= ceiling; ++box)
  {
    const QSize plate = PeekPlateSize(box, textLineHeight, rows);
    if (plate.width() > PeekPlateWidthFraction * cellSize.width()
        || plate.height() > PeekPlateHeightFraction * cellSize.height())
    {
      break;
    }
    best = box;
  }
  return best;
}

QmitkMxNCellOverlay::PeekPlateLayout
QmitkMxNCellOverlay::ComputePeekPlate(const QSize& cellSize, int glyphBox, int pumpedAxis,
                                      int textLineHeight, QmitkMxNPeekRows rows)
{
  PeekPlateLayout layout;
  if (glyphBox < PeekGlyphBoxMin || pumpedAxis >= PeekAxisCount)
  {
    return layout;
  }

  const QSize plateSize = PeekPlateSize(glyphBox, textLineHeight, rows);
  if (plateSize.width() > cellSize.width() || plateSize.height() > cellSize.height())
  {
    return layout;
  }

  layout.plate = QRect(QPoint((cellSize.width() - plateSize.width()) / 2,
                              (cellSize.height() - plateSize.height()) / 2),
                       plateSize);

  const int rowWidth = PeekRowWidth(glyphBox, rows);
  const int rowHeight = PeekRowHeight(glyphBox);
  const int rowLeft = layout.plate.left() + PeekPlatePadX;

  // Each value hugs its own glyph rather than sharing one baseline far below the
  // resting ones - at the row's full height they would sit as far from their
  // glyphs as from the window name, and read as belonging to neither. The band
  // therefore starts right under the resting glyphs.
  const PeekPlateVertical v = PeekPlateVerticalLayout(glyphBox, textLineHeight, rows);
  const int top = layout.plate.top();
  const int rowTop = top + v.firstRowTop;
  const int secondRowTop = top + v.secondRowTop;
  layout.caption = QRect(rowLeft, top + v.captionTop, rowWidth, textLineHeight);
  layout.values = QRect(rowLeft, top + v.valuesTop, rowWidth, v.valuesBottom - v.valuesTop);
  layout.name = QRect(rowLeft, top + v.nameTop, rowWidth, textLineHeight);

  // The pumped glyph grows in place over its neighbours rather than making room
  // among them. Nothing else moves, so the rect a glyph occupies never depends
  // on which axis is pumped: the pointer cannot end up on a neighbour that slid
  // under it, and emphasis looks the same wherever it comes from.
  const int pumpedSide = PeekPumpedSide(glyphBox);
  const int columns = QmitkMxNPeekColumns(rows);
  for (int axis = 0; axis < PeekAxisCount; ++axis)
  {
    const int bandTop = axis < columns ? rowTop : secondRowTop;
    const int nominalLeft =
      rowLeft + PeekPumpOverhang(glyphBox) + (axis % columns) * (glyphBox + PeekGlyphGap);
    layout.glyphs[axis] = axis == pumpedAxis
      ? QRect(nominalLeft + glyphBox / 2 - pumpedSide / 2, bandTop, pumpedSide, pumpedSide)
      : QRect(nominalLeft, bandTop + (rowHeight - glyphBox) / 2, glyphBox, glyphBox);
  }
  return layout;
}

void QmitkMxNCellOverlay::SetSyncPeek(bool visible, std::optional<QmitkMxNSyncAxis> axis, int glyphBox,
                                      QmitkMxNPeekRows rows)
{
  const bool up = visible && glyphBox >= PeekGlyphBoxMin;
  const int slot = up && axis.has_value() ? QmitkMxNSyncAxisToSlot(*axis) : -1;
  const int box = up ? glyphBox : 0;
  if (up == m_SyncPeekVisible && slot == m_SyncPeekAxis && box == m_SyncPeekGlyphBox && rows == m_SyncPeekRows)
  {
    return;
  }
  m_SyncPeekRows = rows;

  const bool wasUp = m_SyncPeekVisible;
  m_SyncPeekVisible = up;
  m_SyncPeekAxis = slot;
  m_SyncPeekGlyphBox = box;
  // A lowered plate fades out where it stood, so the geometry it is painted at
  // outlives the request until the fade has finished.
  if (up)
  {
    m_PeekPaintAxis = slot;
    m_PeekPaintBox = box;
    m_PeekPaintRows = rows;
  }
  else if (m_PeekProgress <= 0.0)
  {
    this->ClearPeekPaintGeometry();
  }
  this->UpdateInteractivity();

  // Changing which axis is emphasised while the peek is up only repaints: the
  // user is comparing axes and a fade between them would read as a flicker.
  if (wasUp == up)
  {
    this->update();
    return;
  }

  const qreal target = up ? 1.0 : 0.0;
  if (m_PeekAnimation.isNull() || !this->AnimationsEnabled())
  {
    this->SetPeekProgress(target);
    this->update();
    return;
  }

  m_PeekAnimation->stop();
  m_PeekAnimation->setDuration(up ? PeekFadeInMs : PeekFadeOutMs);
  m_PeekAnimation->setStartValue(m_PeekProgress);
  m_PeekAnimation->setEndValue(target);
  m_PeekAnimation->start();
  this->update();
}

bool QmitkMxNCellOverlay::IsSyncPeekVisible() const
{
  return m_SyncPeekVisible;
}

std::optional<QmitkMxNSyncAxis> QmitkMxNCellOverlay::SyncPeekAxis() const
{
  return QmitkMxNSyncAxisFromSlot(m_SyncPeekAxis);
}

int QmitkMxNCellOverlay::SyncPeekGlyphBox() const
{
  return m_SyncPeekGlyphBox;
}

QmitkMxNPeekRows QmitkMxNCellOverlay::SyncPeekRows() const
{
  return m_SyncPeekRows;
}

qreal QmitkMxNCellOverlay::PeekProgress() const
{
  return m_PeekProgress;
}

void QmitkMxNCellOverlay::SetPeekProgress(qreal progress)
{
  progress = std::clamp<qreal>(progress, 0.0, 1.0);
  if (qFuzzyCompare(progress, m_PeekProgress))
  {
    return;
  }
  const bool wasPainting = m_PeekProgress > 0.0;
  m_PeekProgress = progress;
  if (m_PeekProgress <= 0.0 && !m_SyncPeekVisible)
  {
    this->ClearPeekPaintGeometry();
  }
  if (wasPainting != (m_PeekProgress > 0.0))
  {
    // The mask doubles as the paint clip, so it has to gain and lose the plate
    // as the fade starts and finishes.
    this->UpdateInteractivity();
  }
  this->update();
}

void QmitkMxNCellOverlay::ClearPeekPaintGeometry()
{
  m_PeekPaintAxis = -1;
  m_PeekPaintBox = 0;
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

  rows.push_back({ NavRow::Kind::Slice, this->NavigatorDepthLabel(), 0.0 });
  if (m_NavigatorExpanded)
  {
    rows.push_back({ NavRow::Kind::InPlaneRight, tr("Horiz."), 0.0 });
    rows.push_back({ NavRow::Kind::InPlaneUp, tr("Vert."), 0.0 });
  }
  if (m_TimeSteps > 1)
  {
    rows.push_back({ NavRow::Kind::Time, tr("Time"), 0.0 });
  }

  // In-plane state is only needed when there is an in-plane row.
  mitk::Point3D origin;
  mitk::Vector3D rightUnit, upUnit;
  double extentRight = 0.0, extentUp = 0.0, rightCoord = 0.0, upCoord = 0.0;
  const bool hasPlane = m_NavigatorExpanded
    && this->NavigatorInPlaneState(origin, rightUnit, upUnit, extentRight, extentUp, rightCoord, upCoord);

  // The label column is only as wide as the widest label, capped so a long
  // plane name cannot eat the track. A fixed column stranded a short label
  // ('Time') far from the track it names while the track ran on to the
  // colorbar, so a row read as belonging to the colorbar instead.
  const QFontMetrics labelMetrics(ReadoutFont(this->font()));
  int labelColumn = 0;
  for (const auto& row : rows)
  {
    labelColumn = std::max(labelColumn, labelMetrics.horizontalAdvance(row.label));
  }
  labelColumn = std::min(labelColumn, qRound(NavLabelMaxFraction * band.width()));

  // Both ends are inset by the knob's radius: the knob is centered on the
  // track's end points, so without the inset it would cover the label at one
  // end and be clipped by the intensity cluster at the other.
  const int trackLeft = band.left() + labelColumn + NavLabelGap + NavKnobHotRadius;
  const int trackWidth = std::max(1, band.right() - NavKnobHotRadius - trackLeft);
  for (std::size_t i = 0; i < rows.size(); ++i)
  {
    const int rowTop = band.top() + static_cast<int>(i) * (NavRowHeight + NavRowGap);
    rows[i].track = QRect(trackLeft, rowTop, trackWidth, NavRowHeight);
    rows[i].labelRect = QRect(band.left(), rowTop, labelColumn, NavRowHeight);

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
  // Stop where the intensity cluster begins. The colorbar is not just the
  // ribbon: its range labels occupy a further TickScaleWidth to the ribbon's
  // left, and a slider running under them makes both unreadable. A cell with no
  // level window paints neither, so there the band takes the full width.
  const int right = m_HasLevelWindow
    ? this->RibbonRect().left() - TickScaleWidth - ReadoutMargin
    : area.right() - ReadoutMargin;
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
  const int steps = static_cast<int>(stepper->GetSteps());
  const int stepperPosition = mitk::SliceNavigationHelper::IsDisplayedSliceInverted(renderer) ? steps - 1 - position : position;
  const int delta = stepperPosition - static_cast<int>(stepper->GetPos());
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

  // This mask clips painting as well as input, and the cell whose barcode is
  // being pointed at is normally masked (the strip sits at the trailing edge,
  // inside the colorbar's reveal margin); leaving the plate out clipped it away
  // in exactly the window the pointer was in. While the mask is up the overlay
  // receives the pointer over the plate, so its own handlers hand plate input to
  // the same arrange-mode path the render window's filter uses. The bumped
  // frame joins the mask for the same reason, as a band along the cell edge.
  const QRect peekPlate = this->SyncPeekPlateRect();
  if (peekPlate.isValid())
  {
    mask += peekPlate;
  }
  if (this->IsArrangeFrameBumped())
  {
    mask += QRegion(this->rect())
            - QRegion(this->rect().adjusted(ArrangeFrameWidth, ArrangeFrameWidth, -ArrangeFrameWidth, -ArrangeFrameWidth));
  }

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
    // Clean view hides the furniture, but while the layout is being arranged
    // the plates and bumped frames are the arrangement itself, not furniture.
    if (!this->IsArranging() && !this->IsArrangeFrameBumped())
    {
      return;
    }
    QPainter painter(this);
    this->PaintArrangeFrame(painter);
    this->PaintSyncPeek(painter);
    return;
  }

  const QRect area = this->RenderWindowRect();
  if (!area.isValid())
  {
    return;
  }

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

  // ---- Bottom-left: navigation (plane label, slice + time) ----
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
        painter.drawText(row.labelRect, Qt::AlignLeft | Qt::AlignVCenter,
                         labelMetrics.elidedText(row.label, Qt::ElideRight,
                                                 row.labelRect.width()));

        const int thickness = rowHot ? NavTrackThickness + 1 : NavTrackThickness;
        painter.fillRect(
          QRect(row.track.left(), centerY - thickness / 2, row.track.width(), thickness),
          Faded(IdleText, rowAlpha));

        const int knobX = row.track.left() + qRound(row.normalized * row.track.width());
        const int knobRadius = rowHot ? NavKnobHotRadius : NavKnobRadius;
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

  this->PaintArrangeFrame(painter);

  // Last, so the plate reads over the furniture as well as over the image.
  this->PaintSyncPeek(painter);
}

void QmitkMxNCellOverlay::PaintArrangeFrame(QPainter& painter)
{
  if (!this->IsArrangeFrameBumped())
  {
    return;
  }
  // The overlay covers the whole cell, frame included, so it paints the bump
  // over the stylesheet border instead of restyling it: a stylesheet write
  // repolishes the cell subtree, far too slow to follow the pointer.
  const qreal half = 0.5 * ArrangeFrameWidth;
  painter.save();
  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.setBrush(Qt::NoBrush);
  QPen pen = m_DropTarget ? QPen(this->SelectionColor(), ArrangeFrameWidth, Qt::DashLine)
                          : QPen(m_Editor->GetArrangeMode()->GetHighlightHue(), ArrangeFrameWidth);
  pen.setJoinStyle(Qt::MiterJoin);
  painter.setPen(pen);
  painter.drawRect(QRectF(this->rect()).adjusted(half, half, -half, -half));
  painter.restore();
}

QColor QmitkMxNCellOverlay::SelectionColor() const
{
  // The theme may colour selections only through item-view rules, which do not
  // reach the palette; it names the colour for widgets that paint their own.
  const QString themed = QmitkIconTheme::GetSelectionColor();
  return themed.isEmpty() ? this->palette().color(QPalette::Highlight) : QColor(themed);
}

QRect QmitkMxNCellOverlay::SyncPeekPlateRect() const
{
  if (!m_SyncPeekVisible && m_PeekProgress <= 0.0)
  {
    return QRect();
  }
  const QRect area = this->RenderWindowRect();
  const PeekPlateLayout layout = ComputePeekPlate(
    area.size(), m_PeekPaintBox, m_PeekPaintAxis, PeekTextLineHeight(this->font()), m_PeekPaintRows);
  return layout.plate.isValid() ? layout.plate.translated(area.topLeft()) : QRect();
}

void QmitkMxNCellOverlay::PaintSyncPeek(QPainter& painter)
{
  if (!m_SyncPeekVisible && m_PeekProgress <= 0.0)
  {
    return;
  }

  const QRect area = this->RenderWindowRect();
  const PeekPlateLayout layout = ComputePeekPlate(
    area.size(), m_PeekPaintBox, m_PeekPaintAxis, PeekTextLineHeight(this->font()), m_PeekPaintRows);
  if (!layout.plate.isValid())
  {
    return;
  }

  // Each window answers for itself: no partner set is computed anywhere, and the
  // plate reads the slots this cell's own strip was given, so an axis reads on
  // the plate exactly as it reads in the strip.
  const auto* utilityWidget = m_Cell->GetUtilityWidget();
  if (nullptr == utilityWidget)
  {
    return;
  }
  const auto axisSlots = utilityWidget->GetSyncBarcodeSlots();
  if (axisSlots.size() != PeekAxisCount)
  {
    return;
  }

  painter.save();
  painter.translate(area.topLeft());
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setOpacity(m_PeekProgress);

  const bool arranging = this->IsArranging();
  const bool selected =
    arranging && m_Editor->GetArrangeMode()->GetSelectedWindowIds().contains(m_Cell->GetWidgetName());
  // Selection gets its own channel, a frame in the theme's selection colour -
  // never a white outline, which the pumped glyph's rings already use for
  // "pointed at".
  painter.setPen(selected ? QPen(this->SelectionColor(), 2) : QPen(PeekPlateBorder, 1));
  painter.setBrush(PeekPlateFill);
  const qreal edge = selected ? 1.0 : 0.5;
  painter.drawRoundedRect(QRectF(layout.plate).adjusted(edge, edge, -edge, -edge),
                          PeekPlateRadius, PeekPlateRadius);

  // Everything resting is painted first, glyphs and values; the pumped glyph and
  // its value come last. They are lifted to the front and may overlap their
  // neighbours - and, with two rows, the other row - so they must be on top.
  const qreal dpr = nullptr != painter.device() ? painter.device()->devicePixelRatioF() : 1.0;
  const auto paintGlyph = [&](int axis)
  {
    const auto& axisSlot = axisSlots[axis];
    const bool synced = axisSlot.color.isValid();
    // The two carriers are kept independent: opacity says whether this window
    // synchronizes the axis, size says which axis the pointer is on. That lets
    // one row be read either way round - the window's whole coupling at a
    // glance, and the pointed-at axis from the pump - instead of emphasis and
    // membership fighting over the same channel. The sticker's rings belong to
    // the size carrier, so they fade with the glyph they outline.
    const qreal weight = synced ? 1.0 : PeekAbsentWeight;
    const QRect box = layout.glyphs[axis];
    const QColor color = synced ? axisSlot.color : PeekAbsentGlyph;
    const int sizePx = qRound(box.width() * dpr);
    painter.setOpacity(m_PeekProgress * weight);
    if (axis == m_PeekPaintAxis)
    {
      const QPixmap sticker = QmitkMxNRenderAxisGlyphSticker(axisSlot.glyph, color, sizePx, dpr);
      if (!sticker.isNull())
      {
        const QSizeF stickerSize = QSizeF(sticker.size()) / dpr;
        painter.drawPixmap(QRectF(box).center() - QPointF(stickerSize.width(), stickerSize.height()) / 2,
                           sticker);
      }
      return;
    }
    const QPixmap glyph = QmitkMxNRenderAxisGlyph(axisSlot.glyph, color, sizePx, dpr);
    if (glyph.isNull())
    {
      return;
    }
    painter.drawPixmap(box.topLeft(), glyph);
  };
  for (int axis = 0; axis < PeekAxisCount; ++axis)
  {
    if (axis != m_PeekPaintAxis)
    {
      paintGlyph(axis);
    }
  }

  // Both lines are elided into the width the glyph row dictates; measuring text
  // into the plate would resize it as the pointed-at axis or the window changes.
  painter.setOpacity(m_PeekProgress);
  painter.setFont(ReadoutFont(this->font()));
  const QFontMetrics metrics(painter.font());
  painter.setPen(ActiveText);
  // The caption names the emphasised axis. Reaching the strip between two glyphs
  // emphasises none; the line stays reserved but empty so the plate never
  // resizes under the pointer.
  if (m_PeekPaintAxis >= 0)
  {
    // In arrange mode the caption stays clear of the button corner,
    // symmetrically so it remains centred over the row.
    const int reserve = arranging
      ? std::max(0, PlateButtonInset + 2 * PlateButtonSize + PlateButtonGap - PeekPlatePadX)
      : 0;
    const QRect caption = layout.caption.adjusted(reserve, 0, -reserve, 0);
    painter.drawText(caption, Qt::AlignHCenter | Qt::AlignVCenter,
                     metrics.elidedText(axisSlots[m_PeekPaintAxis].label, Qt::ElideRight,
                                        caption.width()));
  }

  const QRect closeButton = this->PlateCloseButtonRect();
  const QRect menuButton = this->PlateMenuButtonRect();
  if (closeButton.isValid())
  {
    painter.save();
    painter.setBrush(PlateButtonHover);
    painter.setPen(Qt::NoPen);
    // The disc under the pointer says "this is a button"; without it the glyph
    // alone reads as decoration.
    if (PlateButton::Close == m_PlateHoverButton)
    {
      painter.drawEllipse(QRectF(closeButton.translated(-area.topLeft())));
    }
    if (PlateButton::Menu == m_PlateHoverButton)
    {
      painter.drawEllipse(QRectF(menuButton.translated(-area.topLeft())));
    }

    const QRectF cross = QRectF(closeButton.translated(-area.topLeft())).adjusted(4.5, 4.5, -4.5, -4.5);
    painter.setPen(QPen(ActiveText, 1.5, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(cross.topLeft(), cross.bottomRight());
    painter.drawLine(cross.topRight(), cross.bottomLeft());

    // A vertical ellipsis: three stacked bars would be the slice glyph.
    const QPointF dots = QRectF(menuButton.translated(-area.topLeft())).center();
    painter.setPen(Qt::NoPen);
    painter.setBrush(ActiveText);
    for (const qreal dy : { -4.0, 0.0, 4.0 })
    {
      painter.drawEllipse(dots + QPointF(0.0, dy), 1.4, 1.4);
    }
    painter.restore();
  }

  // The offsets, read straight off the row: each value directly under the glyph
  // it belongs to and in that group's hue - the hue the glyph above it already
  // carries - so the pair reads as one object and the shift is tied to what it
  // is measured from without a word spent saying so. The pointed-at axis grows
  // its value with its glyph, so the answer is largest exactly where the
  // pointer is. Only slice, zoom and pan can be offset, so most of this line is
  // empty and the few values on it are what the eye lands on. A value keeps its
  // own size rather than shrinking to clear its neighbours: three adjacent
  // shifts may crowd, which is rarer than needing to read one.
  QFont valueFont = painter.font();
  QFont pumpedValueFont = valueFont;
  pumpedValueFont.setPointSizeF(PeekPumpScale * valueFont.pointSizeF());
  const auto paintValue = [&](int axis)
  {
    const auto& axisSlot = axisSlots[axis];
    if (axisSlot.offsetText.isEmpty() || !axisSlot.color.isValid())
    {
      return;
    }
    const bool pumped = axis == m_PeekPaintAxis;
    const QFont& font = pumped ? pumpedValueFont : valueFont;
    const QFontMetrics valueMetrics(font);
    const QRect glyphRect = layout.glyphs[axis];
    const int width = valueMetrics.horizontalAdvance(axisSlot.offsetText);
    // A line box is taller than the digits sitting in it. Lifting the box by
    // that slack makes PeekValueGap the gap the eye actually sees, which is what
    // binds the value to its glyph rather than to the window name below.
    const int slack =
      (valueMetrics.height() - valueMetrics.tightBoundingRect(axisSlot.offsetText).height()) / 2;
    const QRect at(glyphRect.center().x() - width / 2,
                   glyphRect.bottom() + PeekValueGap - slack, width, valueMetrics.height());
    if (pumped)
    {
      // The line box is exactly one line tall and exactly as wide as the text,
      // so its top-left plus the ascent is where drawText would put the baseline.
      QmitkMxNPaintStickerText(painter, QPointF(at.left(), at.top() + valueMetrics.ascent()), font,
                               axisSlot.offsetText, axisSlot.color);
      return;
    }
    painter.setFont(font);
    painter.setPen(axisSlot.color);
    painter.drawText(at, Qt::AlignHCenter | Qt::AlignVCenter, axisSlot.offsetText);
  };
  for (int axis = 0; axis < PeekAxisCount; ++axis)
  {
    if (axis != m_PeekPaintAxis)
    {
      paintValue(axis);
    }
  }
  if (m_PeekPaintAxis >= 0)
  {
    paintGlyph(m_PeekPaintAxis);
    painter.setOpacity(m_PeekProgress);
    paintValue(m_PeekPaintAxis);
  }
  painter.setOpacity(m_PeekProgress);
  painter.setFont(valueFont);
  painter.setPen(IdleText);
  painter.drawText(layout.name, Qt::AlignHCenter | Qt::AlignVCenter,
                   metrics.elidedText(m_Editor->CellLabel(m_Cell->GetWidgetName()), Qt::ElideRight,
                                      layout.name.width()));
  painter.restore();
}

void QmitkMxNCellOverlay::mousePressEvent(QMouseEvent* event)
{
  const QPoint position = event->pos();

  if (this->HandlePlateInput(QEvent::MouseButtonPress, event, position))
  {
    event->accept();
    return;
  }

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
      if (NavGrabRect(rows[i].track).contains(position))
      {
        m_NavDragRow = static_cast<int>(i);
        this->ApplyNavigatorRow(rows[i],
          static_cast<double>(position.x() - rows[i].track.left()) / std::max(1, rows[i].track.width()));
        event->accept();
        return;
      }
    }
  }

  // The mask covers passive furniture too (hairline, corners, top strip); a
  // press there is meant for the image underneath, and so are the moves and
  // the release that follow it.
  m_ForwardingGesture = true;
  this->ForwardToRenderWindow(event);
  event->accept();
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
  if (this->HandlePlateInput(QEvent::MouseMove, event, event->pos()))
  {
    event->accept();
    return;
  }

  if (m_ForwardingGesture)
  {
    this->ForwardToRenderWindow(event);
    event->accept();
    return;
  }

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
      if (NavGrabRect(rows[static_cast<std::size_t>(i)].track).contains(event->pos()))
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
  if (this->HandlePlateInput(QEvent::MouseButtonRelease, event, event->pos()))
  {
    event->accept();
    return;
  }

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

  if (m_ForwardingGesture)
  {
    m_ForwardingGesture = Qt::NoButton != event->buttons();
    this->ForwardToRenderWindow(event);
    event->accept();
    return;
  }

  event->ignore();
}

void QmitkMxNCellOverlay::wheelEvent(QWheelEvent* event)
{
  // No furniture scrolls; the wheel belongs to the image under it.
  this->ForwardToRenderWindow(event);
  event->accept();
}

void QmitkMxNCellOverlay::ForwardToRenderWindow(QEvent* event)
{
  auto* renderWindow = m_Cell->GetRenderWindow();
  if (nullptr == renderWindow)
  {
    return;
  }
  const QPointF offset(this->RenderWindowRect().topLeft());

  switch (event->type())
  {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
    {
      const auto* mouseEvent = static_cast<const QMouseEvent*>(event);
      QMouseEvent forwarded(mouseEvent->type(), mouseEvent->position() - offset, mouseEvent->scenePosition(),
                            mouseEvent->globalPosition(), mouseEvent->button(), mouseEvent->buttons(),
                            mouseEvent->modifiers(), mouseEvent->pointingDevice());
      QCoreApplication::sendEvent(renderWindow, &forwarded);
      break;
    }
    case QEvent::Wheel:
    {
      const auto* wheelEvent = static_cast<const QWheelEvent*>(event);
      QWheelEvent forwarded(wheelEvent->position() - offset, wheelEvent->globalPosition(), wheelEvent->pixelDelta(),
                            wheelEvent->angleDelta(), wheelEvent->buttons(), wheelEvent->modifiers(),
                            wheelEvent->phase(), wheelEvent->inverted(), Qt::MouseEventNotSynthesized,
                            wheelEvent->pointingDevice());
      QCoreApplication::sendEvent(renderWindow, &forwarded);
      break;
    }
    default:
      break;
  }
}

namespace
{
  /** Selects a spin box's whole value when a click gives it focus, as Qt
   *  already does for Tab: a click otherwise leaves the cursor where it landed,
   *  usually behind the last decimal, where the box's validator rejects every
   *  typed digit. Deferred, because the press that focuses the box places the
   *  cursor only after the focus event. */
  class SelectAllOnClickFocus : public QObject
  {
  public:
    explicit SelectAllOnClickFocus(QAbstractSpinBox* box)
      : QObject(box)
    {
      box->installEventFilter(this);
    }

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
      if (QEvent::FocusIn == event->type()
          && Qt::MouseFocusReason == static_cast<QFocusEvent*>(event)->reason())
      {
        auto* box = static_cast<QAbstractSpinBox*>(watched);
        QTimer::singleShot(0, box, [box]() { box->selectAll(); });
      }
      return false;
    }
  };

  /** Shows an entry popup with its top-left at 'globalPosition', shifted back
   *  onto the screen: unlike a QMenu, a moved popup frame is not kept visible
   *  by Qt, and a menu opened near a screen edge would push it off. */
  void ShowEntryPopupAt(QWidget* popup, const QPoint& globalPosition)
  {
    QPoint topLeft = globalPosition;
    if (const auto* screen = QGuiApplication::screenAt(globalPosition); nullptr != screen)
    {
      const QRect available = screen->availableGeometry();
      const QSize size = popup->sizeHint();
      topLeft.setX(std::clamp(topLeft.x(), available.left(), std::max(available.left(), available.right() - size.width())));
      topLeft.setY(std::clamp(topLeft.y(), available.top(), std::max(available.top(), available.bottom() - size.height())));
    }
    popup->move(topLeft);
    popup->show();
  }
}

void QmitkMxNCellOverlay::OpenNumericEntry(std::optional<QPoint> globalPosition)
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
  new SelectAllOnClickFocus(levelBox);
  new SelectAllOnClickFocus(windowBox);

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

  popup->adjustSize();
  if (globalPosition.has_value())
  {
    ShowEntryPopupAt(popup, *globalPosition);
  }
  else
  {
    const QRect readout = this->WindowLevelRect();
    popup->move(this->mapToGlobal(QPoint(readout.left(), readout.top() - popup->sizeHint().height() - 4)));
    popup->show();
    this->PinWhileOpen(popup);
  }
  levelBox->setFocus();
  levelBox->selectAll();
}

void QmitkMxNCellOverlay::OpenCoordinateEntry(std::optional<QPoint> globalPosition)
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
    new SelectAllOnClickFocus(box);
    worldLayout->addWidget(box);
    worldBoxes[static_cast<std::size_t>(axis)] = box;
  }
  layout->addRow(tr("World (mm)"), worldRow);

  std::array<QSpinBox*, 3> indexBoxes{};
  if (hasIndex)
  {
    auto* indexRow = new QWidget(popup);
    auto* indexLayout = new QHBoxLayout(indexRow);
    indexLayout->setContentsMargins({});
    for (int axis = 0; axis < 3; ++axis)
    {
      auto* box = new QSpinBox(indexRow);
      box->setRange(-100000, 100000);
      box->setValue(static_cast<int>(std::lround(index[axis])));
      new SelectAllOnClickFocus(box);
      indexLayout->addWidget(box);
      indexBoxes[static_cast<std::size_t>(axis)] = box;
    }
    layout->addRow(tr("Voxel index"), indexRow);
  }

  // Both rows show where the crosshair actually landed, not what was typed: the
  // edited unit and the other one stay in step, and a move the navigation
  // clamps or snaps shows as such.
  const auto refreshFromCrosshair = [this, worldBoxes, indexBoxes, hasIndex]()
  {
    const mitk::Point3D landed = this->CrosshairWorld();
    for (int axis = 0; axis < 3; ++axis)
    {
      auto* box = worldBoxes[static_cast<std::size_t>(axis)];
      const QSignalBlocker blocker(box);
      box->setValue(landed[axis]);
    }
    if (!hasIndex || m_TopNode.IsNull() || nullptr == m_TopNode->GetData()
        || nullptr == m_TopNode->GetData()->GetGeometry())
    {
      return;
    }
    mitk::Point3D voxel;
    m_TopNode->GetData()->GetGeometry()->WorldToIndex(landed, voxel);
    for (int axis = 0; axis < 3; ++axis)
    {
      auto* box = indexBoxes[static_cast<std::size_t>(axis)];
      const QSignalBlocker blocker(box);
      box->setValue(static_cast<int>(std::lround(voxel[axis])));
    }
  };

  const auto commitWorld = [this, worldBoxes, refreshFromCrosshair]()
  {
    mitk::Point3D position;
    for (int axis = 0; axis < 3; ++axis)
    {
      position[axis] = worldBoxes[static_cast<std::size_t>(axis)]->value();
    }
    this->NavigatorSetCrosshair(position);
    refreshFromCrosshair();
  };
  for (auto* box : worldBoxes)
  {
    connect(box, &QDoubleSpinBox::editingFinished, this, commitWorld);
  }

  if (hasIndex)
  {
    const auto commitIndex = [this, indexBoxes, refreshFromCrosshair]()
    {
      mitk::Point3D voxel;
      for (int axis = 0; axis < 3; ++axis)
      {
        voxel[axis] = indexBoxes[static_cast<std::size_t>(axis)]->value();
      }
      this->NavigatorSetVoxelIndex(voxel);
      refreshFromCrosshair();
    };
    for (auto* box : indexBoxes)
    {
      connect(box, &QSpinBox::editingFinished, this, commitIndex);
    }
  }

  popup->adjustSize();
  if (globalPosition.has_value())
  {
    ShowEntryPopupAt(popup, *globalPosition);
  }
  else
  {
    const QRect coord = this->CoordinateLineRect();
    popup->move(this->mapToGlobal(QPoint(coord.left(), coord.top() - popup->sizeHint().height() - 4)));
    popup->show();
    this->PinWhileOpen(popup);
  }
  worldBoxes[0]->setFocus();
  worldBoxes[0]->selectAll();
}

void QmitkMxNCellOverlay::PinWhileOpen(QWidget* popup)
{
  if (m_Proximity.isNull())
  {
    return;
  }
  m_Proximity->SetPinned(true);
  // The controller is the receiver, so the unpin is dropped if it dies first.
  connect(popup, &QObject::destroyed, m_Proximity.data(),
          [proximity = m_Proximity.data()]() { proximity->SetPinned(false); });
}

void QmitkMxNCellOverlay::OpenColormapMenu()
{
  if (m_TopNode.IsNull())
  {
    return;
  }

  // No parent, like every menu of this overlay: the menu's event loop also
  // delivers queued cross-thread calls, and a layout applied by one (a REST
  // request) destroys this overlay, which would delete the menu from under
  // its own stack frame.
  QMenu menu;
  const auto& names = mitk::LookupTable::typenameList;
  for (std::size_t i = 0; i < names.size(); ++i)
  {
    auto* action = menu.addAction(QString::fromStdString(names[i]));
    action->setData(static_cast<int>(i));
  }

  const QPointer<QmitkMxNCellOverlay> self(this);
  const ScopedProximityPin pin(m_Proximity);
  auto* chosen = menu.exec(this->mapToGlobal(this->ColormapChipRect().bottomLeft()));
  if (nullptr == chosen || self.isNull())
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

void QmitkMxNCellOverlay::PopulateDirectionMenu(QMenu* menu)
{
  const auto windowId = m_Cell->GetWidgetName();
  const std::pair<const char*, mitk::AnatomicalPlane> planes[] = {
    { "Axial", mitk::AnatomicalPlane::Axial },
    { "Coronal", mitk::AnatomicalPlane::Coronal },
    { "Sagittal", mitk::AnatomicalPlane::Sagittal },
  };
  for (const auto& [label, plane] : planes)
  {
    auto* action = menu->addAction(tr(label));
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
}

void QmitkMxNCellOverlay::OpenDirectionPicker(const QPoint& globalPosition)
{
  QMenu menu;  // No parent, see OpenColormapMenu.
  this->PopulateDirectionMenu(&menu);
  const ScopedProximityPin pin(m_Proximity);
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
    if (sliceSteps > 0 && mitk::SliceNavigationHelper::IsDisplayedSliceInverted(renderer))
    {
      slicePosition = sliceSteps - 1 - slicePosition;
    }
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
  if (watched == m_Cell && this->HandleCellDrag(event))
  {
    return true;
  }

  // Arrange mode takes plate input from the render window's own stream: the
  // overlay stays mouse-transparent there, so VTK keeps every pointer event
  // that does not start on a plate.
  if (watched == m_Cell->GetRenderWindow() && this->IsArranging())
  {
    const QPoint offset = this->RenderWindowRect().topLeft();
    switch (event->type())
    {
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonDblClick:
      case QEvent::MouseMove:
      case QEvent::MouseButtonRelease:
      {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (this->HandlePlateInput(event->type(), mouseEvent, mouseEvent->position().toPoint() + offset))
        {
          return true;
        }
        break;
      }
      case QEvent::Leave:
        this->ClearPlateHover();
        break;
      case QEvent::ToolTip:
      {
        auto* helpEvent = static_cast<QHelpEvent*>(event);
        if (this->ShowPlateButtonToolTip(helpEvent->pos() + offset, helpEvent->globalPos()))
        {
          return true;
        }
        break;
      }
      default:
        break;
    }
  }

  if (watched == m_Cell->GetRenderWindow())
  {
    // The right button doubles as the zoom / windowing gesture, so only a
    // click without drag is a context-menu request, and that is known only at
    // the release. Qt's own request cannot tell: on UNIX it arrives with the
    // press (QStyleHints::contextMenuTrigger) and would swallow every gesture.
    // So every mouse-reason request is consumed and the menu opens from the
    // release instead, deferred so that VTK still sees the release. The menu
    // stays available in clean view: it is transient (right-click only, not
    // passive furniture) and it is the way back - it carries the clean-view
    // toggle, which is otherwise unreachable once the utility strip hides.
    switch (event->type())
    {
      case QEvent::Resize:
      case QEvent::Move:
        // The mask is built from the render window's rect, which can change
        // without the cell resizing (a docked utility row shown or hidden).
        this->UpdateInteractivity();
        this->update();
        break;
      case QEvent::MouseButtonPress:
      {
        const auto* mouseEvent = static_cast<const QMouseEvent*>(event);
        if (Qt::RightButton == mouseEvent->button())
        {
          m_RightPressPosition = mouseEvent->position().toPoint();
          m_RightPressArmed = true;
        }
        break;
      }
      case QEvent::MouseButtonRelease:
      {
        const auto* mouseEvent = static_cast<const QMouseEvent*>(event);
        if (Qt::RightButton == mouseEvent->button() && m_RightPressArmed)
        {
          m_RightPressArmed = false;
          if ((mouseEvent->position().toPoint() - m_RightPressPosition).manhattanLength()
              < QApplication::startDragDistance())
          {
            const QPoint globalPosition = mouseEvent->globalPosition().toPoint();
            QTimer::singleShot(0, this, [this, globalPosition]() { this->OpenContextMenu(globalPosition); });
          }
        }
        break;
      }
      case QEvent::ContextMenu:
      {
        // A keyboard request (Menu key, Shift+F10) has no press to wait for;
        // Qt only delivers it to the focused render window, so it is deliberate.
        const auto* contextEvent = static_cast<const QContextMenuEvent*>(event);
        if (QContextMenuEvent::Keyboard == contextEvent->reason())
        {
          this->OpenContextMenu(contextEvent->globalPos());
        }
        return true;
      }
      default:
        break;
    }
  }

  return QmitkOverlayWidget::eventFilter(watched, event);
}

bool QmitkMxNCellOverlay::IsArranging() const
{
  const auto* arrangeMode = m_Editor->GetArrangeMode();
  return nullptr != arrangeMode && arrangeMode->IsActive() && m_SyncPeekVisible;
}

bool QmitkMxNCellOverlay::HandlePlateInput(QEvent::Type type, QMouseEvent* event, const QPoint& position)
{
  if (!this->IsArranging())
  {
    return false;
  }
  auto* arrangeMode = m_Editor->GetArrangeMode();

  switch (type)
  {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    {
      // A double click arrives in place of the second press; taking it like a
      // press keeps a quick second click on the plate a selection gesture.
      if (!this->SyncPeekPlateRect().contains(position))
      {
        return false;
      }
      if (Qt::LeftButton == event->button() && this->PlateCloseButtonRect().contains(position))
      {
        m_Editor->RequestLayoutEditor(QmitkMxNMultiWidget::LayoutEditorRequest::Hide);
        return true;
      }
      if (Qt::LeftButton == event->button() && this->PlateMenuButtonRect().contains(position))
      {
        this->OpenPlateMenu(this->mapToGlobal(position));
        return true;
      }
      m_PlatePressActive = true;
      m_PlatePressPosition = position;
      m_PlatePressButton = event->button();
      m_PlateDragArmed = arrangeMode->PressCell(m_Cell->GetWidgetName(), event->button(), event->modifiers());
      return true;
    }
    case QEvent::MouseMove:
    {
      if (!m_PlatePressActive)
      {
        // Only a free pointer points at glyphs; one dragging a VTK gesture
        // across the plate is doing something else.
        if (Qt::NoButton == event->buttons())
        {
          this->UpdatePlateHover(position);
        }
        return false;
      }
      if (m_PlateDragArmed
          && (position - m_PlatePressPosition).manhattanLength() >= QApplication::startDragDistance())
      {
        m_PlatePressActive = false;
        m_PlateDragArmed = false;
        // Owned by the editor, not this overlay: the drag's event loop can
        // deliver a layout apply that destroys the overlay mid-drag.
        auto* drag = new QDrag(m_Editor);
        drag->setMimeData(arrangeMode->CreateDragMimeData());
        arrangeMode->ReleaseCell(true);
        drag->exec(Qt::CopyAction);
      }
      return true;
    }
    case QEvent::MouseButtonRelease:
    {
      if (!m_PlatePressActive)
      {
        return false;
      }
      // A right click on a plate is about the arrangement, so it opens the
      // plate's menu rather than the cell's, from the release for the reason
      // given in eventFilter. A right drag has already become the ask-mode
      // drag and leaves nothing to open.
      const bool rightClick = Qt::RightButton == m_PlatePressButton && Qt::RightButton == event->button()
        && (position - m_PlatePressPosition).manhattanLength() < QApplication::startDragDistance();
      m_PlatePressActive = false;
      m_PlateDragArmed = false;
      arrangeMode->ReleaseCell(false);
      if (rightClick)
      {
        const QPoint globalPosition = this->mapToGlobal(position);
        QTimer::singleShot(0, this, [this, globalPosition]() { this->OpenPlateMenu(globalPosition); });
      }
      return true;
    }
    default:
      return false;
  }
}

int QmitkMxNCellOverlay::PlateGlyphAt(const QPoint& position) const
{
  const QRect area = this->RenderWindowRect();
  const PeekPlateLayout layout = ComputePeekPlate(
    area.size(), m_PeekPaintBox, m_PeekPaintAxis, PeekTextLineHeight(this->font()), m_PeekPaintRows);
  if (!layout.plate.isValid())
  {
    return -1;
  }
  const QPoint local = position - area.topLeft();
  if (m_PeekPaintAxis >= 0 && layout.glyphs[m_PeekPaintAxis].contains(local))
  {
    return m_PeekPaintAxis;
  }
  for (int axis = 0; axis < PeekAxisCount; ++axis)
  {
    if (layout.glyphs[axis].contains(local))
    {
      return axis;
    }
  }
  return -1;
}

void QmitkMxNCellOverlay::UpdatePlateHover(const QPoint& position)
{
  if (!this->SyncPeekPlateRect().contains(position))
  {
    this->ClearPlateHover();
    return;
  }

  if (!m_PlateHovered)
  {
    m_PlateHovered = true;
    this->update();  // the buttons show on the plate under the pointer
  }
  const PlateButton button = this->PlateCloseButtonRect().contains(position) ? PlateButton::Close
                             : this->PlateMenuButtonRect().contains(position) ? PlateButton::Menu
                                                                             : PlateButton::None;
  if (button != m_PlateHoverButton)
  {
    m_PlateHoverButton = button;
    this->update();
  }

  // The emphasis latches across the gaps between glyphs, as on the strip: it
  // moves when another glyph claims it and is dropped only off the plate.
  const int slot = this->PlateGlyphAt(position);
  const auto axis = QmitkMxNSyncAxisFromSlot(slot);
  if (!axis.has_value() || slot == m_PlateHoverAxis)
  {
    return;
  }
  m_PlateHoverAxis = slot;

  const std::string group = m_Editor->ResolveCellAxisGroup(m_Cell->GetWidgetName(), *axis);
  QStringList members;
  QColor hue;
  if (!group.empty())
  {
    members = m_Editor->CellsSharingAxis(QString::fromStdString(group), *axis);
    try
    {
      hue = m_Editor->GetSyncGroupColor(group);
    }
    catch (const mitk::Exception&)
    {
      members.clear();
    }
  }
  m_Editor->GetArrangeMode()->SetHighlight(QmitkMxNArrangeMode::HighlightSource::Plate, axis, members, hue);
}

void QmitkMxNCellOverlay::ClearPlateHover()
{
  if (!m_PlateHovered)
  {
    return;
  }
  m_PlateHovered = false;
  m_PlateHoverAxis = -1;
  m_PlateHoverButton = PlateButton::None;
  m_Editor->GetArrangeMode()->ClearHighlight(QmitkMxNArrangeMode::HighlightSource::Plate);
  this->update();
}

QRect QmitkMxNCellOverlay::PlateCloseButtonRect() const
{
  if (!m_PlateHovered || !this->IsArranging())
  {
    return QRect();
  }
  const QRect plate = this->SyncPeekPlateRect();
  if (!plate.isValid())
  {
    return QRect();
  }
  return QRect(plate.right() - PlateButtonInset - PlateButtonSize + 1, plate.top() + PlateButtonInset,
               PlateButtonSize, PlateButtonSize);
}

bool QmitkMxNCellOverlay::ShowPlateButtonToolTip(const QPoint& position, const QPoint& globalPosition)
{
  if (this->PlateCloseButtonRect().contains(position))
  {
    QToolTip::showText(globalPosition, tr("Close the layout editor"), this);
    return true;
  }
  if (this->PlateMenuButtonRect().contains(position))
  {
    QToolTip::showText(globalPosition, tr("Group and selection actions"), this);
    return true;
  }
  return false;
}

void QmitkMxNCellOverlay::contextMenuEvent(QContextMenuEvent* event)
{
  // The press under a mouse request was forwarded to the render window or
  // taken by the plate, and either opens its menu from the release. The
  // overlay never has focus, so no keyboard request arrives here.
  if (QContextMenuEvent::Mouse == event->reason())
  {
    event->accept();
    return;
  }
  QmitkOverlayWidget::contextMenuEvent(event);
}

bool QmitkMxNCellOverlay::event(QEvent* event)
{
  if (QEvent::ToolTip == event->type())
  {
    auto* helpEvent = static_cast<QHelpEvent*>(event);
    if (this->ShowPlateButtonToolTip(helpEvent->pos(), helpEvent->globalPos()))
    {
      return true;
    }
  }
  return QmitkOverlayWidget::event(event);
}

QRect QmitkMxNCellOverlay::PlateMenuButtonRect() const
{
  const QRect close = this->PlateCloseButtonRect();
  return close.isValid() ? close.translated(-(PlateButtonSize + PlateButtonGap), 0) : QRect();
}

void QmitkMxNCellOverlay::OpenPlateMenu(const QPoint& globalPosition)
{
  auto* arrangeMode = m_Editor->GetArrangeMode();
  const QString windowId = m_Cell->GetWidgetName();

  std::vector<QmitkMxNMultiWidget::SyncGroupInfo> infos;
  try
  {
    infos = m_Editor->GetSyncGroupInfos();
  }
  catch (const mitk::Exception&)
  {
    return;  // mid-layout-change; there is nothing stable to offer
  }
  const auto swatch = [](const QColor& color)
  {
    QPixmap pixmap(12, 12);
    pixmap.fill(color.isValid() ? color : QColor(Qt::gray));
    return QIcon(pixmap);
  };

  QMenu menu;  // No parent, see OpenColormapMenu.

  // Replace is what the group cards' "Add selected windows" does, so the two
  // ways of adding mean the same.
  auto* addMenu = menu.addMenu(tr("Add to group"));
  for (const auto& info : infos)
  {
    const auto group = QString::fromStdString(info.id);
    connect(addMenu->addAction(swatch(info.color), QString::fromStdString(info.displayName)), &QAction::triggered,
            this, [arrangeMode, group, windowId]()
            { arrangeMode->RequestAssign(group, windowId, QmitkMxNGroupJoinMode::Replace); });
  }
  addMenu->setEnabled(!addMenu->isEmpty());

  // A window can be on different groups on different axes, and removal (like
  // the cards') is per group, so every group it is on is offered.
  auto* removeMenu = menu.addMenu(tr("Remove from group"));
  std::vector<std::string> linked;
  for (int slot = 0; slot < PeekAxisCount; ++slot)
  {
    const auto group = m_Editor->ResolveCellAxisGroup(windowId, *QmitkMxNSyncAxisFromSlot(slot));
    if (!group.empty() && std::find(linked.begin(), linked.end(), group) == linked.end())
    {
      linked.push_back(group);
    }
  }
  for (const auto& group : linked)
  {
    const auto info = std::find_if(infos.begin(), infos.end(), [&group](const auto& i) { return i.id == group; });
    const QString name = QString::fromStdString(info != infos.end() ? info->displayName : group);
    const QColor color = info != infos.end() ? info->color : QColor();
    const auto groupId = QString::fromStdString(group);
    connect(removeMenu->addAction(swatch(color), name), &QAction::triggered, this,
            [arrangeMode, groupId, windowId]() { arrangeMode->RequestRemove(groupId, windowId); });
  }
  removeMenu->setEnabled(!removeMenu->isEmpty());

  menu.addSeparator();
  auto* clear = menu.addAction(tr("Clear selection"));
  clear->setEnabled(!arrangeMode->GetSelectedWindowIds().isEmpty());
  connect(clear, &QAction::triggered, arrangeMode, &QmitkMxNArrangeMode::ClearSelection);

  menu.exec(globalPosition);
}

bool QmitkMxNCellOverlay::IsArrangeFrameBumped() const
{
  const auto* arrangeMode = m_Editor->GetArrangeMode();
  if (nullptr == arrangeMode || !arrangeMode->IsActive())
  {
    return false;
  }
  return m_DropTarget
    || (arrangeMode->GetHighlightHue().isValid()
        && arrangeMode->GetHighlightedWindowIds().contains(m_Cell->GetWidgetName()));
}

bool QmitkMxNCellOverlay::HandleCellDrag(QEvent* event)
{
  const auto* arrangeMode = m_Editor->GetArrangeMode();
  if (nullptr == arrangeMode || !arrangeMode->IsActive())
  {
    return false;
  }

  // Only group drags are taken. Anything else is left unaccepted, which lets
  // Qt offer it to the cell's ancestors - a file drag still reaches the editor
  // area, a data-node drag still finds no taker - exactly as without arrange
  // mode.
  const auto setDropTarget = [this](bool target)
  {
    if (target != m_DropTarget)
    {
      m_DropTarget = target;
      this->UpdateInteractivity();
      this->update();
    }
  };

  switch (event->type())
  {
    case QEvent::DragEnter:
    case QEvent::DragMove:
    {
      auto* dragEvent = static_cast<QDragMoveEvent*>(event);
      if (!dragEvent->mimeData()->hasFormat(QmitkMxNGroupMimeType))
      {
        return false;
      }
      dragEvent->acceptProposedAction();
      setDropTarget(true);
      return true;
    }
    case QEvent::DragLeave:
      setDropTarget(false);
      return false;
    case QEvent::Drop:
    {
      setDropTarget(false);
      auto* dropEvent = static_cast<QDropEvent*>(event);
      if (!dropEvent->mimeData()->hasFormat(QmitkMxNGroupMimeType))
      {
        return false;
      }
      // The overlay is re-checked after the ask-mode menu, for the reason
      // given in OpenColormapMenu.
      const QPointer<QmitkMxNCellOverlay> self(this);
      const auto mode = QmitkMxNResolveJoinMode(dropEvent->mimeData(), dropEvent->modifiers(),
                                                m_Cell->mapToGlobal(dropEvent->position().toPoint()));
      if (self.isNull())
      {
        return true;
      }
      if (mode.has_value())
      {
        m_Editor->GetArrangeMode()->RequestAssign(
          QString::fromUtf8(dropEvent->mimeData()->data(QmitkMxNGroupMimeType)), m_Cell->GetWidgetName(), *mode);
        dropEvent->acceptProposedAction();
      }
      return true;
    }
    default:
      return false;
  }
}

void QmitkMxNCellOverlay::resizeEvent(QResizeEvent* event)
{
  QmitkOverlayWidget::resizeEvent(event);
  // The mask is geometry: nothing else rebuilds it while the pointer rests.
  this->UpdateInteractivity();
  this->update();
  m_Editor->RequestSyncPeekRefresh();
}

void QmitkMxNCellOverlay::OpenContextMenu(const QPoint& globalPosition)
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer)
  {
    return;
  }
  const auto windowId = m_Cell->GetWidgetName();

  QMenu menu;  // No parent, see OpenColormapMenu.

  this->PopulateDirectionMenu(menu.addMenu(tr("View direction")));

  // The furniture's own entry points - the Data button, the W/L readout, the
  // navigator's coordinate line - are pointer-only and hidden until revealed;
  // these make them reachable from a keyboard-opened menu too. Each opens a
  // popup of its own, so it waits until this menu has closed.
  menu.addSeparator();

  auto* dataAction = menu.addAction(tr("Data..."));
  connect(dataAction, &QAction::triggered, this, [this, globalPosition]()
  {
    QTimer::singleShot(0, this, [this, globalPosition]()
    {
      if (auto* utilityWidget = m_Cell->GetUtilityWidget(); nullptr != utilityWidget)
      {
        utilityWidget->ShowDataSelection(globalPosition);
      }
    });
  });

  auto* levelWindowAction = menu.addAction(tr("Set level/window..."));
  levelWindowAction->setEnabled(m_TopNode.IsNotNull());
  connect(levelWindowAction, &QAction::triggered, this, [this, globalPosition]()
  {
    QTimer::singleShot(0, this, [this, globalPosition]() { this->OpenNumericEntry(globalPosition); });
  });

  auto* coordinateAction = menu.addAction(tr("Go to coordinate..."));
  connect(coordinateAction, &QAction::triggered, this, [this, globalPosition]()
  {
    QTimer::singleShot(0, this, [this, globalPosition]() { this->OpenCoordinateEntry(globalPosition); });
  });

  menu.addSeparator();

  auto* reinitAction = menu.addAction(tr("Fit group views to visible data"));
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

  // The MxN utility strip covers the built-in render-window menu, so the two
  // controls of that menu that still mean something for a cell whose
  // arrangement the layout editor owns are offered here instead.
  // No icons on the checkable entries: a style draws the check mark and the icon
  // in the same column, so an icon silently replaces the tick and the state
  // stops being readable. The strip carries the icons; the menu carries state.
  auto* maximizeAction = menu.addAction(tr("Maximized"));
  maximizeAction->setCheckable(true);
  maximizeAction->setChecked(m_Editor->GetMaximizedCell() == windowId);
  connect(maximizeAction, &QAction::toggled, this, [this, windowId](bool maximized)
  {
    m_Editor->SetMaximizedCell(maximized ? windowId : QString());
  });

  auto* crosshairAction = menu.addAction(tr("Show crosshair"));
  crosshairAction->setCheckable(true);
  crosshairAction->setChecked(m_Editor->GetCrosshairVisibility());
  connect(crosshairAction, &QAction::toggled, this, [this](bool visible)
  {
    m_Editor->SetCrosshairVisibility(visible);
  });

  auto* cleanViewAction = menu.addAction(tr("Clean view"));
  cleanViewAction->setShortcut(QmitkMxNMultiWidget::CleanViewShortcut());
  cleanViewAction->setShortcutVisibleInContextMenu(true);
  cleanViewAction->setCheckable(true);
  cleanViewAction->setChecked(m_Editor->IsCleanView());
  connect(cleanViewAction, &QAction::toggled, this, [this](bool cleanView)
  {
    m_Editor->SetCleanView(cleanView);
  });

  menu.addSeparator();

  auto* editorAction = menu.addAction(tr("Open layout editor"));
  connect(editorAction, &QAction::triggered, this, [this]()
  {
    m_Editor->RequestLayoutEditor(QmitkMxNMultiWidget::LayoutEditorRequest::Show);
  });

  auto* expandedNavigatorAction = menu.addAction(tr("Expanded navigator"));
  expandedNavigatorAction->setCheckable(true);
  expandedNavigatorAction->setChecked(m_Editor->IsNavigatorExpanded());
  connect(expandedNavigatorAction, &QAction::toggled, this, [this](bool expanded)
  {
    m_Editor->SetNavigatorExpanded(expanded);
  });

  menu.exec(globalPosition);
}
