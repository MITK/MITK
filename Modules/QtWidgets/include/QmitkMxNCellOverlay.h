/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNCellOverlay_h
#define QmitkMxNCellOverlay_h

#include <MitkQtWidgetsExports.h>

#include <QmitkOverlayWidget.h>
#include <QmitkRenderWindowProximity.h>

#include <mitkDataNode.h>
#include <mitkLevelWindow.h>
#include <mitkVector.h>

#include <vtkSmartPointer.h>
#include <vtkType.h>

#include <QColor>
#include <QImage>
#include <QPointer>
#include <QRect>
#include <QString>

#include <vector>

class QmitkMxNMultiWidget;
class QmitkRenderWindowWidget;
class QPropertyAnimation;
class vtkRenderWindow;

/**
 * \brief The composite viewport-furniture overlay of one MxN cell.
 *
 * One Qt overlay per cell paints all furniture over the render window and
 * owns its sub-region hit testing (rather than stacking one full-rect
 * overlay per surface, which would multiply z-order and hit-test ambiguity).
 *
 * The two concerns are split by side so that no fact shares a spot with an
 * unrelated one and no readout reads as belonging to the wrong control
 * (Gestalt proximity):
 *
 *   - Navigation lives bottom-left: line 1 is the view-plane label (drawn
 *     here, in the same layer as the slice readout, so the two align; the
 *     VTK corner annotation is blanked at the cell), tracking the cell's
 *     current view direction; line 2 is `slice N/max` plus the group-identity
 *     dot, and, for time-resolved data, a clock glyph and `t n/m`.
 *   - Intensity lives on the right edge and bottom-right: the colorbar (the
 *     cell's actual LUT) is a 2 px passive strip that widens into a legend +
 *     control on reveal (a value tick scale with the level marked in hue, an
 *     on-bar colormap picker), and the `W <window> L <level>` readout sits
 *     bottom-right beside it. Dragging the colorbar body shifts the level and
 *     dragging its ends moves the window bounds, routed through the editor's
 *     synchronized per-renderer level-window path so group members follow;
 *     double-clicking the readout opens numeric entry (the by-value setter).
 *   - The bottom hairline carries the slice-position tick (a solid bar); for
 *     time-resolved data the time-step position rides the same hairline as a
 *     distinct triangle marker.
 *
 * The group-identity dot is a solid hue only when every dimension the cell is
 * synchronized on names one group (mono-group); a cell spanning more than one
 * group shows a distinct complex marker instead of a single hue that would
 * misrepresent the state - the per-dimension truth lives in the sync barcode,
 * the seams, and the layout editor.
 *
 * Passive readouts are always-on and faint (the level/window readout honors
 * the MxN preference and every readout honors clean-view). The interactive
 * furniture reveals as one coordinated frame while the pointer is in the cell
 * - quiet and translucent at rest, fully opaque and expressive under the
 * pointer - and eases in and collapses together (opacity + offset), rather
 * than region by region.
 *
 * The overlay is mouse-transparent while idle. While a furniture region is
 * active it takes input only over the visible furniture bounds (widget
 * mask), so VTK interaction is never shadowed elsewhere; it feeds the
 * proximity controller as an event source, keeping reveal state alive while
 * the pointer is over the furniture itself. On proximity to the bottom edge
 * the standard slice-navigation slider is hosted over the image; the top edge
 * shows a thin strip for the auto-hidden utility toolbar and reveals it on
 * approach. A right-click (without drag - the zoom/windowing gestures stay
 * untouched) opens the cell's context menu.
 *
 * Values refresh on the cell's VTK render-end events, coalesced to the next
 * event-loop cycle - not on a timer.
 *
 * This is a Qt layer on top of the VTK scene: the colorbar, readouts, and
 * plane label are deliberately not VTK annotations, keeping one code path for
 * the passive and active layers.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNCellOverlay : public QmitkOverlayWidget
{
  Q_OBJECT

  // 0 = interactive frame collapsed, 1 = fully revealed. Animated by
  // QPropertyAnimation so the reveal eases in / out as one coordinated frame.
  Q_PROPERTY(qreal revealProgress READ RevealProgress WRITE SetRevealProgress)

public:

  /**
   * \brief Attach the overlay to a cell.
   *
   * \param cell       The cell to decorate; also the Qt parent (the overlay
   *                   dies with it). Must not be null and must already hold
   *                   its render window.
   * \param editor     The owning editor, used for the synchronized
   *                   level-window / lookup-table writes and group lookups.
   *                   Must not be null and must outlive the cell.
   * \param proximity  The cell's proximity controller; the overlay registers
   *                   its furniture regions and follows the emitted states.
   *                   Must not be null and must outlive the overlay.
   *
   * \throws mitk::Exception on a null argument or a cell without a render
   *         window.
   */
  QmitkMxNCellOverlay(QmitkRenderWindowWidget* cell,
                      QmitkMxNMultiWidget* editor,
                      QmitkRenderWindowProximity* proximity);

  ~QmitkMxNCellOverlay() override;

  /**
   * \brief Preference-controlled default visibility of the corner readout.
   *        When off, the readout follows the proximity reveal like all other
   *        furniture instead of being always-on.
   */
  void SetReadoutVisible(bool visible);

  /** \brief Clean-view mode: while set, the overlay paints nothing at all. */
  void SetCleanView(bool cleanView);

  /**
   * \brief Navigator mode for this cell: compact (the slice slider only) or
   *        expanded (the full 3D crosshair as three axis sliders plus the
   *        coordinate entry). Mirrored editor-wide by the owning editor.
   */
  void SetNavigatorExpanded(bool expanded);
  bool IsNavigatorExpanded() const;

  /**
   * \brief Programmatic navigator drives, also used by the painted sliders and
   *        the coordinate entry. Slice goes through the slice stepper and the
   *        crosshair through the world position, both by firing the same
   *        display-action events an interaction would - so a linked cell
   *        follows its group and an unlinked cell moves alone. Exposed for
   *        headless verification.
   */
  void NavigatorSetSlice(int position);
  void NavigatorSetCrosshair(const mitk::Point3D& worldPosition);
  void NavigatorSetVoxelIndex(const mitk::Point3D& voxelIndex);
  void NavigatorMoveInPlane(double rightMm, double upMm);

  /** \brief The depth (slice) row label: "Slice - <plane>" for an orthogonal
   *         view, a generic "Slice" otherwise. Exposed for verification. */
  QString NavigatorDepthLabel() const;

  /**
   * \brief The plane label currently shown bottom-left, tracking the cell's
   *        view direction (empty before the first refresh or for a view with
   *        no fixed anatomical name). Exposed for headless verification of the
   *        reorientation tracking.
   */
  QString PlaneLabel() const;

  qreal RevealProgress() const;
  void SetRevealProgress(qreal progress);

protected:

  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

  /** \brief Adds render-window context-menu handling to the base's
   *         parent-resize tracking. */
  bool eventFilter(QObject* watched, QEvent* event) override;

  /** \brief Clears the hovered element when the pointer leaves the furniture. */
  void leaveEvent(QEvent* event) override;

private:

  enum class DragMode
  {
    None,
    Level,       // ribbon body: shift the level
    UpperBound,  // ribbon top end: move the upper window bound
    LowerBound   // ribbon bottom end: move the lower window bound
  };

  /**
   * \brief The interactive element under the pointer, if any. Reveal is driven
   *        by proximity (the frame eases in when the pointer nears the
   *        furniture), but an element only *highlights* - turns opaque and
   *        offers interaction - while the pointer is actually over it, so the
   *        cue never lands on an element the pointer is merely near.
   */
  enum class HoverTarget
  {
    None,
    Ribbon,
    Colormap,
    WindowLevel,
    PlaneLabel,
    Coordinate
  };

  /** \brief The interactive element currently under the pointer. */
  HoverTarget HoverAt(const QPoint& pos) const;

  /** \brief The render window's rect in cell (== overlay) coordinates. */
  QRect RenderWindowRect() const;

  /** \brief The interactive (revealed, inset) colorbar rect used for the drag,
   *         tick scale, and colormap-chip anchor. */
  QRect RibbonRect() const;

  /** \brief The view-plane label, bottom-left line 1 (navigation). */
  QRect PlaneLabelRect() const;

  /** \brief The `slice N/max` readout plus group dot, bottom-left line 2. */
  QRect SliceReadoutRect() const;

  /** \brief The `W <window> L <level>` readout, bottom-right beside the colorbar. */
  QRect WindowLevelRect() const;

  /** \brief The colormap picker chip on the colorbar (active layer only),
   *         below the top strip so the two never overlap. */
  QRect ColormapChipRect() const;

  /** \brief Hot region along the cell's top edge revealing the toolbar. */
  QRect TopStripRect() const;

  /** \brief The plane label for the cell's current view direction, resolved
   *         live from the renderer (empty for a view with no fixed anatomical
   *         name). */
  QString ResolvePlaneLabel() const;

  /** \brief One painted navigator slider row. */
  struct NavRow
  {
    enum class Kind
    {
      Slice,          // depth: the cell's slice stepper
      InPlaneRight,   // in-plane horizontal crosshair move
      InPlaneUp,      // in-plane vertical crosshair move
      Time            // the global time stepper
    };
    Kind kind = Kind::Slice;
    QString label;
    double normalized = 0.0;  // current knob position, 0..1
    QRect track;
    QRect labelRect;          // the row's label column, left of its track
  };

  /** \brief The navigator rows for the current mode and data (compact = slice
   *         [+ time]; expanded = depth, in-plane H, in-plane V [+ time]),
   *         with their tracks laid out and knob positions resolved. */
  std::vector<NavRow> NavigatorRows() const;

  /** \brief The bottom band the navigator sliders occupy. */
  QRect NavigatorBandRect() const;

  /** \brief The click-to-edit coordinate line, expanded mode only. */
  QRect CoordinateLineRect() const;

  /**
   * \brief The cell's current in-plane frame: plane origin, unit right / up
   *        axes (oblique-safe, from the current world plane geometry), the
   *        in-plane extents (mm), and the crosshair's current in-plane
   *        coordinates. Returns false when no plane geometry is available.
   */
  bool NavigatorInPlaneState(mitk::Point3D& origin, mitk::Vector3D& rightUnit,
                             mitk::Vector3D& upUnit, double& extentRight, double& extentUp,
                             double& rightCoord, double& upCoord) const;

  /** \brief Apply a knob drag on a row: map the normalized position to the
   *         row's action (slice step, in-plane move, or time step). */
  void ApplyNavigatorRow(const NavRow& row, double normalized);

  /** \brief The cell's current crosshair world position (via the editor). */
  mitk::Point3D CrosshairWorld() const;

  void OpenCoordinateEntry();

  void OpenContextMenu(const QPoint& globalPosition);

  /** \brief Whether a passive readout paints: always-on items honor only
   *         clean-view; the level/window readout additionally honors its
   *         preference (following the reveal when the preference is off). */
  bool IsPassiveVisible(bool honorReadoutPreference) const;

  /** \brief True while the pointer is anywhere in the cell (any region left
   *         Idle): the trigger for the one coordinated interactive reveal. */
  bool IsFrameRevealed() const;

  /** \brief (Re)drive the reveal animation from the current frame state. */
  void UpdateReveal();

  /** \brief Style-driven reduced-motion check; false falls back to instant. */
  bool AnimationsEnabled() const;

  void OnProximityStateChanged(QmitkRenderWindowProximity::RegionId id,
                               QmitkRenderWindowProximity::State state);

  /**
   * \brief Sync mouse transparency and the input/paint mask with the current
   *        reveal state: input only over visible furniture while active,
   *        fully transparent otherwise. A running drag pins the interactive
   *        state so a mid-drag collapse cannot break the implicit grab.
   */
  void UpdateInteractivity();

  /** \brief Coalesce VTK render-end notifications into one refresh per cycle. */
  void ScheduleValueRefresh();

  /** \brief Re-read node, level/window, and LUT; repaint on change. */
  void RefreshValues();

  /** \brief Topmost image node visible in this cell's renderer, or null. */
  mitk::DataNode::Pointer ResolveTopImageNode() const;

  void RebuildLutStrip();

  void OpenNumericEntry();
  void OpenColormapMenu();

  /** \brief Axial/Coronal/Sagittal picker opened by clicking the plane label;
   *         sets the cell's view direction (and propagates to its group). */
  void OpenDirectionPicker(const QPoint& globalPosition);

  /** \brief World value per dragged pixel, from the drag-start window. */
  double DragScale() const;

  /**
   * \brief Present this cell's render window synchronously during a drag.
   *        A QVTKOpenGLNativeWidget only schedules its on-screen paint on a
   *        VTK render, and the coalesced render request is starved by a
   *        continuous drag's move stream - so a live drag must both regenerate
   *        (ForceImmediateUpdate) and force the widget to repaint now.
   */
  void RenderCellNow();

  static void OnVtkRenderEnd(vtkObject* caller, unsigned long eventId,
                             void* clientData, void* callData);

  QmitkRenderWindowWidget* m_Cell;
  QmitkMxNMultiWidget* m_Editor;
  QPointer<QmitkRenderWindowProximity> m_Proximity;
  vtkSmartPointer<vtkRenderWindow> m_VtkRenderWindow;
  unsigned long m_VtkObserverTag = 0;

  QmitkRenderWindowProximity::RegionId m_RibbonRegion = -1;
  QmitkRenderWindowProximity::RegionId m_WindowLevelRegion = -1;
  QmitkRenderWindowProximity::RegionId m_PlaneLabelRegion = -1;
  QmitkRenderWindowProximity::RegionId m_BottomRegion = -1;
  QmitkRenderWindowProximity::RegionId m_TopRegion = -1;
  QmitkRenderWindowProximity::State m_RibbonState = QmitkRenderWindowProximity::State::Idle;
  QmitkRenderWindowProximity::State m_WindowLevelState = QmitkRenderWindowProximity::State::Idle;
  QmitkRenderWindowProximity::State m_PlaneLabelState = QmitkRenderWindowProximity::State::Idle;
  QmitkRenderWindowProximity::State m_BottomState = QmitkRenderWindowProximity::State::Idle;
  QmitkRenderWindowProximity::State m_TopState = QmitkRenderWindowProximity::State::Idle;

  // 0 collapsed .. 1 fully revealed, driven by m_RevealAnimation.
  qreal m_RevealProgress = 0.0;
  QPointer<QPropertyAnimation> m_RevealAnimation;
  bool m_FrameRevealed = false;

  bool m_ReadoutVisible = true;
  bool m_CleanView = false;
  bool m_RefreshPending = false;

  mitk::DataNode::Pointer m_TopNode;
  mitk::LevelWindow m_LevelWindow;
  bool m_HasLevelWindow = false;
  QImage m_LutStrip;
  vtkMTimeType m_LutMTime = 0;

  DragMode m_DragMode = DragMode::None;
  QPoint m_LastDragPosition;
  double m_DragScale = 1.0;

  HoverTarget m_Hover = HoverTarget::None;
  int m_HoverNavRow = -1;   // navigator slider row under the pointer, or -1

  unsigned int m_SlicePosition = 0;
  unsigned int m_SliceSteps = 0;
  unsigned int m_TimePosition = 0;
  unsigned int m_TimeSteps = 0;

  bool m_NavigatorExpanded = false;
  int m_NavDragRow = -1;                    // index into NavigatorRows() while dragging
  QPoint m_RightPressPosition;              // context-menu drag suppression

};

#endif
