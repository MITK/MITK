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

#include <QmitkMxNSyncDimension.h>
#include <QmitkOverlayWidget.h>
#include <QmitkRenderWindowProximity.h>

#include <mitkDataNode.h>
#include <mitkLevelWindow.h>
#include <mitkVector.h>

#include <vtkSmartPointer.h>
#include <vtkType.h>

#include <QColor>
#include <QEvent>
#include <QImage>
#include <QPointer>
#include <QRect>
#include <QSize>
#include <QString>

#include <array>
#include <optional>
#include <vector>

class QmitkMxNMultiWidget;
class QmitkRenderWindowWidget;
class QFont;
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
 *     current view direction; line 2 is `slice N/max` and, for time-resolved
 *     data, a clock glyph and `t n/m`.
 *   - Intensity lives on the right edge and bottom-right: the colorbar (the
 *     cell's actual LUT) is a 2 px passive strip that widens into a legend +
 *     control on reveal (a value tick scale with the level marked in hue, an
 *     on-bar colormap picker), and the `W <window> L <level>` readout sits
 *     bottom-right beside it. Dragging the colorbar body shifts the level and
 *     dragging its ends moves the window bounds, routed through the editor's
 *     synchronized per-renderer level-window path so group members follow;
 *     clicking the readout opens numeric entry (the by-value setter).
 *   - The bottom hairline carries the slice-position tick (a solid bar); for
 *     time-resolved data the time-step position rides the same hairline as a
 *     distinct triangle marker.
 *
 * Passive readouts are always-on and faint (the level/window readout honors
 * the MxN preference and every readout honors clean-view). The interactive
 * furniture reveals as one coordinated frame when the pointer nears any piece
 * of it or the top edge, so the centre of the image stays uncovered. It is
 * quiet and translucent at rest, fully opaque and expressive under the
 * pointer, and eases in and collapses together (opacity + offset) rather
 * than region by region.
 *
 * The overlay is mouse-transparent while idle. While a furniture region is
 * active it takes input only over the visible furniture bounds (widget
 * mask), so VTK interaction is never shadowed elsewhere; it feeds the
 * proximity controller as an event source, keeping reveal state alive while
 * the pointer is over the furniture itself. The navigator is a band of
 * painted slider rows above the bottom-left readouts (compact: slice, plus
 * time for time-resolved data; expanded: depth, in-plane horizontal and
 * vertical, a click-to-edit coordinate line, plus time); it shows while the
 * frame is revealed. While the frame is collapsed, a thin strip along the top
 * edge marks the auto-hidden utility toolbar, which reveals and collapses
 * with the frame. A right-click (without drag - the zoom/windowing gestures
 * stay untouched) opens the cell's context menu.
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

  // 0 = the sync peek plate is down, 1 = fully faded in.
  Q_PROPERTY(qreal peekProgress READ PeekProgress WRITE SetPeekProgress)

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

  /** \brief Clean-view mode: while set, the overlay paints none of the viewport
   *         furniture. In arrange mode the sync peek plate and the bumped frame
   *         still paint: they belong to the arrangement, not the furniture. */
  void SetCleanView(bool cleanView);

  /**
   * \brief Navigator mode for this cell: compact (the slice slider, plus
   *        time for time-resolved data) or expanded (the full 3D crosshair
   *        as three axis sliders plus the coordinate entry). Mirrored
   *        editor-wide by the owning editor.
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
   *
   *        NavigatorSetSlice takes the displayed slice index, which follows
   *        the image's own index axis for the view direction; it is converted
   *        to the stepper position, which can run the opposite way
   *        (see mitk::SliceNavigationHelper::IsSliceIndexInverted).
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

  /** \brief The number of synchronization axes a peek plate shows, matching the
   *         sync barcode's slot count. */
  static constexpr int PeekAxisCount = QmitkMxNSyncAxisCount;

  /** \brief The legible range of a peek plate's glyph box side. */
  static constexpr int PeekGlyphBoxMin = 20;
  static constexpr int PeekGlyphBoxMax = 40;

  /**
  * \brief Show or hide this cell's sync peek plate at the layout-wide 'glyphBox'
  *        side and glyph arrangement 'rows', emphasising 'axis' - or none of
  *        them without one, which is how the plate looks while the pointer rests
  *        on the barcode between two glyphs. The geometry is handed in rather
  *        than derived here: every plate in the layout must share one, or the
  *        rows stop being comparable.
  */
  void SetSyncPeek(bool visible, std::optional<QmitkMxNSyncAxis> axis, int glyphBox, QmitkMxNPeekRows rows);

  /** \brief Whether this cell's peek plate is up. Exposed for verification. */
  bool IsSyncPeekVisible() const;

  /** \brief The axis this cell's peek emphasises, if any. Exposed for
   *         verification. */
  std::optional<QmitkMxNSyncAxis> SyncPeekAxis() const;

  /** \brief The glyph box this cell's peek was given, or 0. Exposed for
   *         verification. */
  int SyncPeekGlyphBox() const;

  /** \brief The glyph arrangement this cell's peek was given. Exposed for
   *         verification. */
  QmitkMxNPeekRows SyncPeekRows() const;

  /** \brief Where a peek plate and its parts land in a cell of 'cellSize'. */
  struct PeekPlateLayout
  {
    QRect plate;                              // invalid when the cell cannot host a plate
    std::array<QRect, PeekAxisCount> glyphs;  // axis order; the pumped one is the large rect
    QRect caption;                            // the pointed-at axis name, above the row
    QRect values;                             // the per-axis offsets, under the first row
    QRect name;                               // the window name, below the values
  };

  /**
  * \brief The plate geometry for one cell: the plate centred in 'cellSize', the
  *        eight glyph rects for 'glyphBox' with 'pumpedAxis' enlarged around its
  *        own slot, and the text rects. Every other glyph keeps its slot, so
  *        the pumped one may overlap its neighbours. A negative 'pumpedAxis' lays
  *        the row out with nothing emphasised. 'rows' lays the glyphs out in one
  *        row or two. Returns an invalid plate when the cell cannot host one.
  *        Static so the geometry is testable without a realized overlay.
  *
  *   The plate reserves the pumped glyph's overhang and all three text lines -
  *   axis name, offset values, window name - whatever the pumped axis is and
  *   whatever this window is offset by, so its rect depends only on 'cellSize',
  *   'glyphBox', 'textLineHeight' and 'rows': switching axes while the peek is up
  *   moves and resizes nothing.
  */
  static PeekPlateLayout ComputePeekPlate(const QSize& cellSize, int glyphBox,
                                          int pumpedAxis, int textLineHeight, QmitkMxNPeekRows rows);

  /**
  * \brief The largest glyph box a cell of 'cellSize' can host within the plate's
  *        share of the cell with the glyphs in 'rows', or 0 below the legible
  *        floor. The multi widget calls this per cell and arrangement to derive
  *        the one geometry the whole layout uses.
  */
  static int MaxPeekGlyphBox(const QSize& cellSize, int textLineHeight, QmitkMxNPeekRows rows);

  /**
  * \brief The height of one peek text line for a widget whose font is
  *        'baseFont'. The plate's lines are set in the peripheral readout font,
  *        which the multi widget cannot measure on its own: it passes its own
  *        font, the one every cell inherits.
  */
  static int PeekTextLineHeight(const QFont& baseFont);

  qreal PeekProgress() const;
  void SetPeekProgress(qreal progress);

  /** \brief The peek plate's rect in overlay coordinates, invalid while the
   *         plate is down. Exposed for verification. */
  QRect SyncPeekPlateRect() const;

  /** \brief The plate's close button in overlay coordinates: valid only in
   *         arrange mode while the pointer is on this cell's plate. Exposed
   *         for verification. */
  QRect PlateCloseButtonRect() const;

  /** \brief The plate's menu button, left of the close button, valid under
   *         the same condition. Exposed for verification. */
  QRect PlateMenuButtonRect() const;

  /** \brief Whether this cell's frame is bumped: arrange mode is on and the
   *         cell shares the pointed-at synchronization, or a group drag hovers
   *         it. Exposed for verification. */
  bool IsArrangeFrameBumped() const;

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

  /** \brief A resize in arrange mode re-resolves the layout's shared glyph box. */
  void resizeEvent(QResizeEvent* event) override;

  /** \brief The plate's menu and button tooltips while the plate is in the
   *         mask and the overlay, not the render window, gets the pointer. */
  void contextMenuEvent(QContextMenuEvent* event) override;
  bool event(QEvent* event) override;

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
    QRect track{};
    QRect labelRect{};        // the row's label column, left of its track
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

  /** \brief Opens the coordinate entry above the coordinate line, or at
   *         'globalPosition' when given (the context menu passes where it
   *         was opened, which is where the user is looking). */
  void OpenCoordinateEntry(std::optional<QPoint> globalPosition = std::nullopt);

  void OpenContextMenu(const QPoint& globalPosition);

  /** \brief Whether a passive readout paints: always-on items honor only
   *         clean-view; the level/window readout additionally honors its
   *         preference (following the reveal when the preference is off). */
  bool IsPassiveVisible(bool honorReadoutPreference) const;

  /** \brief True outside clean view while any furniture region or the top
   *         strip is Active, i.e. the pointer is within its proximity zone:
   *         the trigger for the one coordinated interactive reveal. */
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

  /**
   * \brief Paint the sync peek plate over the image. Deliberately not routed
   *        through QmitkMxNSyncBarcodeWidget::PaintInto: that renderer lays out
   *        a uniform grid, while the plate enlarges one glyph in place. The
   *        artwork stays shared through QmitkMxNRenderAxisGlyph.
   */
  void PaintSyncPeek(QPainter& painter);

  bool IsArranging() const;

  /** \brief The cell frame, bumped to bold for a cell sharing the pointed-at
   *         synchronization or targeted by a group drag; painted in clean view
   *         too. */
  void PaintArrangeFrame(QPainter& painter);

  /** \brief The colour this theme marks selections with. */
  QColor SelectionColor() const;

  /** \brief The plate's menu: add the window (or the selection it belongs to)
   *         to a group, remove it from one of the groups it is on, or clear the
   *         selection. */
  void OpenPlateMenu(const QPoint& globalPosition);

  /** \brief A context-menu request at 'position' (overlay coordinates): opens
   *         the plate menu for a click on the plate. Returns whether the plate
   *         took the request. */
  bool HandlePlateContextMenu(const QPoint& position, const QPoint& globalPosition);

  /** \brief Show the tooltip of the plate button at 'position', if any. */
  bool ShowPlateButtonToolTip(const QPoint& position, const QPoint& globalPosition);

  /**
  * \brief Arrange-mode input on the plate, from wherever it arrives: the render
  *        window's event filter while the overlay is mouse-transparent, the
  *        overlay's own handlers while other furniture makes it take input and
  *        the plate is in its mask. Returns whether the event was consumed.
  *
  *   Presses that start on the plate are taken, with the moves and the release
  *   that follow them; a press anywhere else passes through untouched, and so
  *   does every button-less move - those only update which glyph is pointed at.
  */
  bool HandlePlateInput(QEvent::Type type, QMouseEvent* event, const QPoint& position);

  /** \brief The slot of the plate glyph under 'position' (overlay coordinates):
   *         the pumped glyph first, as it lies over its neighbours, then the
   *         resting ones; -1 over no glyph. */
  int PlateGlyphAt(const QPoint& position) const;

  /** \brief Track the pointer on the plate: which glyph it points at (latched
   *         across the gaps) and whether it is on the plate at all. */
  void UpdatePlateHover(const QPoint& position);
  void ClearPlateHover();

  /** \brief Group drags onto the cell in arrange mode; every other drag is
   *         left to propagate as if the cell accepted none. */
  bool HandleCellDrag(QEvent* event);


  /** \brief Coalesce VTK render-end notifications into one refresh per cycle. */
  void ScheduleValueRefresh();

  /** \brief Re-read node, level/window, and LUT; repaint on change. */
  void RefreshValues();

  /** \brief Topmost image node visible in this cell's renderer, or null. */
  mitk::DataNode::Pointer ResolveTopImageNode() const;

  void RebuildLutStrip();

  /** \brief Opens the level/window entry above the readout, or at
   *         'globalPosition' when given (see 'OpenCoordinateEntry'). */
  void OpenNumericEntry(std::optional<QPoint> globalPosition = std::nullopt);
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

  // The sync peek: whether the plate is up, which axis it emphasises (-1 for
  // none), the layout-wide glyph box it was handed, and the fade driven by
  // m_PeekAnimation.
  bool m_SyncPeekVisible = false;
  int m_SyncPeekAxis = -1;
  int m_SyncPeekGlyphBox = 0;
  QmitkMxNPeekRows m_SyncPeekRows = QmitkMxNPeekRows::One;
  qreal m_PeekProgress = 0.0;
  QPointer<QPropertyAnimation> m_PeekAnimation;

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

  unsigned int m_SlicePosition = 0;  // displayed index along the image axis, not the stepper position
  unsigned int m_SliceSteps = 0;
  unsigned int m_TimePosition = 0;
  unsigned int m_TimeSteps = 0;

  bool m_NavigatorExpanded = false;
  int m_NavDragRow = -1;                    // index into NavigatorRows() while dragging
  QPoint m_RightPressPosition;              // context-menu drag suppression

  // Arrange mode on the plate: a press that started there (and may grow into a
  // drag), the pointer's stay on the plate with its latched glyph, and a group
  // drag hovering the cell.
  bool m_PlatePressActive = false;
  bool m_PlateDragArmed = false;
  QPoint m_PlatePressPosition;
  bool m_PlateHovered = false;
  int m_PlateHoverAxis = -1;
  bool m_DropTarget = false;
  enum class PlateButton
  {
    None,
    Close,
    Menu
  };
  PlateButton m_PlateHoverButton = PlateButton::None;

};

#endif
