/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNMultiWidget_h
#define QmitkMxNMultiWidget_h

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkAbstractMultiWidget.h>
#include <QmitkMxNSyncBarcodeWidget.h>
#include <QmitkMxNSyncDimension.h>
#include <QmitkSynchronizedNodeSelectionWidget.h>
#include <QmitkSynchronizedWidgetConnector.h>

// mitk core
#include <mitkDisplayActionEventFunctions.h>
#include <mitkException.h>
#include <mitkVector.h>

#include <nlohmann/json.hpp>

#include <QColor>
#include <QKeySequence>
#include <QPointer>
#include <QRectF>

#include <array>
#include <functional>
#include <initializer_list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

class QmitkMxNArrangeMode;
class QmitkRenderWindowProximity;
class QDialog;
class QLabel;
class QProgressBar;
class QSplitter;
class QTimer;

namespace mitk
{
  class BaseRenderer;
  class LevelWindow;
  class LookupTable;
}

/**
* \brief Thrown by 'QmitkMxNMultiWidget::ApplyLayout' when it is called while
*        another 'ApplyLayout' on the same editor is still in progress.
*
*   The condition is transient: the same call succeeds once the running apply
*   has returned.
*/
class MITKQTWIDGETS_EXPORT QmitkMxNLayoutBusyException : public mitk::Exception
{
public:
  mitkExceptionClassMacro(QmitkMxNLayoutBusyException, mitk::Exception);
};

/**
* \brief The 'QmitkMxNMultiWidget' is a 'QmitkAbstractMultiWidget' that is used to display multiple render windows at once.
*        Render windows can dynamically be added and removed to change the layout of the multi widget. This
*        is done by using the 'SetLayout'-function to define a layout. This will automatically add or remove
*        the appropriate number of render window widgets.
*
*        In addition to layout management, the widget owns the lifecycle of selection
*        synchronization groups: see 'AddSynchronizationGroup', 'SetSynchronizationGroup',
*        and the 'SyncGroupAdded' signal. Layout configurations can be persisted and
*        restored via 'SaveLayout' / 'LoadLayout'.
*/
class MITKQTWIDGETS_EXPORT QmitkMxNMultiWidget : public QmitkAbstractMultiWidget
{
  Q_OBJECT

public:

  QmitkMxNMultiWidget(QWidget* parent = nullptr,
                      Qt::WindowFlags f = {},
                      const QString& multiWidgetName = "mxn");

  ~QmitkMxNMultiWidget();

  void InitializeMultiWidget() override;

  using QmitkAbstractMultiWidget::RemoveRenderWindowWidget;

  /**
  * \brief Remove the cell, and with it the renderer-specific level/window and
  *        lookup table this editor wrote for it (see
  *        'RemoveCellRendererProperties').
  */
  void RemoveRenderWindowWidget(const QString& widgetName) override;

  /**
  * \brief Editor-scoped synchronization macro over the four broadcast
  *        navigation dimensions (pan, zoom, slice, crosshair).
  *
  *   Enabling links every cell of this editor into one shared group per
  *   navigation dimension (cells created later join automatically while the
  *   macro is active); disabling removes every cell's link for those four
  *   dimensions. The macro rewrites the per-cell links only - unlike
  *   'SetSyncLink' it never converges cell states, preserving the classic
  *   toggle behavior of coupling views in place. Render windows of other
  *   editors are unaffected in both directions. Windowing and LUT links are
  *   left as they are (a new cell links both to the default group), and time
  *   stays application-global.
  */
  void Synchronize(bool synchronized) override;

  QmitkRenderWindow* GetRenderWindow(const QString& widgetName) const override;
  QmitkRenderWindow* GetRenderWindow(const mitk::AnatomicalPlane& orientation) const override;

  void SetActiveRenderWindowWidget(RenderWindowWidgetPointer activeRenderWindowWidget) override;

  /**
  * \brief Initialize the active render windows of the MxNMultiWidget to the given geometry.
  *
  * \param geometry       The geometry to be used to initialize / update the
  *                       active render window's time and slice navigation controller.
  * \param resetCamera    If true, the camera and crosshair will be reset to the default view (centered, no zoom).
  *                       If false, the current crosshair position and the camera zoom will be stored and reset
  *                       after the reference geometry has been updated.
  */
  void InitializeViews(const mitk::TimeGeometry* geometry, bool resetCamera) override;

  /**
  * \brief Forward the given time geometry to all base renderers, so that they can store it as their
  *        interaction reference geometry.
  *        This will update the alignment status of the reference geometry for each base renderer.
  *        For more details, see 'BaseRenderer::SetInteractionReferenceGeometry'.
  *        Overridem from 'QmitkAbstractMultiWidget'.
  */
  void SetInteractionReferenceGeometry(const mitk::TimeGeometry* referenceGeometry) override;

  /**
  * \brief Returns true if the render windows are coupled; false if not.
  *
  * For the MxNMultiWidget the render windows are typically decoupled.
  */
  bool HasCoupledRenderWindows() const override;

  void SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName) override;
  const mitk::Point3D GetSelectedPosition(const QString& widgetName) const override;

  void SetCrosshairVisibility(bool visible) override;
  bool GetCrosshairVisibility() const override;
  void SetCrosshairGap(unsigned int gapSize) override;

  void ResetCrosshair() override;

  void SetWidgetPlaneMode(QmitkCrosshairRotationMode mode) override;

  mitk::SliceNavigationController* GetTimeNavigationController();

  void EnableCrosshair();
  void DisableCrosshair();

  using GroupSyncIndexType = int;

  /**
  * \brief Create a new selection synchronization group with the given index.
  *
  *   Idempotent: calling twice with the same index is a no-op (the existing
  *   connector is preserved, no signal is re-emitted).
  *   On creation the connector's selection is seeded from the data storage's
  *   non-helper, non-hidden nodes and 'SyncGroupAdded(index)' is emitted once.
  *
  *   This is the canonical creation API; callers should go through it rather
  *   than mutating 'm_SynchronizedWidgetConnectors' directly.
  *
  * \param index   The 1-based group index. Must be >= 1.
  * \param name    Optional bare group label to record in the engine's group-
  *                name registry. When empty (the default), the registry
  *                receives the conventional auto-generated label: 'main' for
  *                index 1, otherwise 'g_<index>'. Idempotent calls (the group
  *                already exists) leave the previously recorded name in
  *                place.
  *
  * \pre  index >= 1                       (otherwise mitk::Exception)
  * \pre  GetDataStorage() != nullptr      (otherwise mitk::Exception)
  *
  * \throws mitk::Exception on precondition violation.
  */
  void AddSynchronizationGroup(const GroupSyncIndexType index, const std::string& name = std::string());

  /**
  * \brief Remove a synchronization group entirely, by its string id.
  *
  *   Clears every cell's ties to the group - each dimension link naming it and
  *   any cell whose data selection names it (those cells revert to the default
  *   group) - and drops a leftover registry entry, so a group created empty via
  *   the "+" button (which the per-member reclaim leaves in place) also
  *   disappears. The group's display name and color are dropped as well, so a
  *   later group created with the same id starts clean. The default group
  *   (engine index 1, see GetDefaultSyncGroupName) is never removable: the
  *   call is a no-op for it.
  *   A no-op as well for an id that names no group. Emits SyncLinksChanged
  *   once.
  */
  void RemoveSynchronizationGroup(const std::string& id);

  /**
  * \brief Move a synchronized node selection widget to the group with the given index.
  *
  *   The group is auto-created via 'AddSynchronizationGroup' if it does not
  *   yet exist. After the call, 'synchronizedWidget' is connected to (and only
  *   to) the connector for 'index'. Calling with the widget's current group
  *   does not double-Connect; it only re-runs 'SynchronizeWidget' so cached
  *   state propagates to the widget.
  *
  *   This is the canonical move API; callers should not bypass it.
  *
  * \param synchronizedWidget    The widget to move. Must not be null.
  * \param index                 The 1-based target group index. Must be >= 1.
  *
  * \pre  synchronizedWidget != nullptr           (otherwise mitk::Exception)
  * \pre  index >= 1                              (otherwise mitk::Exception, propagated from Add)
  * \pre  GetDataStorage() != nullptr             (otherwise mitk::Exception, propagated from Add
  *                                                 when the auto-create path triggers)
  *
  * \throws mitk::Exception on precondition violation.
  */
  void SetSynchronizationGroup(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget, const GroupSyncIndexType index);

  /**
  * \brief Move a cell's data-selection group to the group named 'group',
  *        addressing selection by the same string id the other axes use.
  *
  *   Selection is single-valued per cell, so this is a move: the cell leaves
  *   its previous selection group. A selection connector for 'group' is
  *   allocated on first use (mapping the string id to a free engine index) and
  *   reclaimed once its last member leaves - unless it came from a layout
  *   document or the default seed, which persist. No-op for an unknown cell.
  */
  void SetCellSelectionGroup(const QString& windowId, const std::string& group);

  /**
  * \brief Move a cell back to the default data-selection group (clearing a
  *        deliberate assignment). No-op for an unknown cell.
  */
  void ClearCellSelectionGroup(const QString& windowId);

  /**
  * \brief The string id of a cell's current data-selection group (empty for an
  *        unknown cell or an unregistered index).
  */
  std::string GetCellSelectionGroup(const QString& windowId) const;

  /**
  * \brief Returns the smallest positive index not already used by an existing
  *        synchronization group.
  *
  *   Useful for callers that want to allocate a fresh group without colliding
  *   with the set of existing groups (e.g. the "+" button in the per-cell
  *   utility widget, or external automation).
  */
  GroupSyncIndexType NextFreeSyncGroupIndex() const;

  /**
  * \brief Offset modifier of a synchronization link, typed per dimension:
  *        `int` slice steps for `Slice`, a multiplicative factor (`double`,
  *        > 0) for `Zoom`, an in-plane world-mm vector for `Pan`.
  *        `std::monostate` means "no offset" (the dimension's identity).
  */
  using SyncOffset = std::variant<std::monostate, int, double, mitk::Vector2D>;

  /** \brief Group membership and offset of one cell for one dimension. */
  struct SyncLinkState
  {
    std::string group;
    SyncOffset offset;
  };

  /**
  * \brief Link a cell to a named synchronization group for one dimension.
  *
  *   Cells linked to the same group for the same dimension are synchronized
  *   on that dimension only. On joining, the cell is converged to the
  *   group's reference - the live state of the group's seed cell (the
  *   pre-order first member) - combined with the given offset; the seed
  *   itself is never converged. Convergence is skipped while the involved
  *   render windows have no world geometry yet; use 'ReconvergeSyncGroup'
  *   once they do. `Crosshair` links carry no convergence bookkeeping
  *   (propagation is absolute: the crosshair is one world point that every
  *   member resolves into its own slice, so an offset has nothing to be
  *   relative to). An `Orientation` join aligns the cell to the
  *   group's plane. `Windowing` / `Lut` joins do not converge: the cell keeps
  *   its own value until the group's next change propagates.
  *
  * \param windowId   Canonical window id of the cell. Must name an existing cell.
  * \param dimension  The synchronization dimension to link.
  * \param group      Group name (URL-segment-safe pattern, shared namespace
  *                   across all dimensions).
  * \param offset     Offset modifier; must match the dimension's offset type
  *                   (or `std::monostate` for the identity offset). Only
  *                   `Slice`, `Zoom`, and `Pan` accept an offset.
  *
  * \throws mitk::Exception on an unknown window, a malformed group name, or
  *         an offset that the dimension does not accept.
  */
  void SetSyncLink(const QString& windowId, QmitkMxNSyncDimension dimension,
                   const std::string& group, const SyncOffset& offset = {});

  /**
  * \brief Remove a cell's link for one dimension (back to the unsynced
  *        singleton default). No-op if the cell is not linked.
  *
  * \throws mitk::Exception on an unknown window.
  */
  void ClearSyncLink(const QString& windowId, QmitkMxNSyncDimension dimension);

  /**
  * \brief The cell's link state for one dimension; empty if unlinked.
  *        For linked `Slice` / `Zoom` / `Pan` the offset always carries the
  *        dimension's typed value (identity when never set).
  */
  std::optional<SyncLinkState> GetSyncLink(const QString& windowId, QmitkMxNSyncDimension dimension) const;

  /**
  * \brief Notify the sync furniture (per-cell barcodes, frame colors, the
  *        layout editor) of a link-state change by emitting 'SyncLinksChanged'.
  *        The mutators ('SetSyncLink' / 'ClearSyncLink' / 'SetCellSelectionGroup'
  *        ...) stay silent so a batch can settle first; the caller invokes this
  *        once when the batch is done (as 'ApplyLayout' and the layout editor do).
  */
  void RefreshSyncControls();

  /**
  * \brief One cell's offset on one dimension in words: slice in signed steps,
  *        zoom as a factor, pan as whole millimetres. Empty for the neutral
  *        offset and for a dimension that carries none, so a surface shows an
  *        offset only where one was actually authored. Static and public so
  *        every surface that shows offsets - the barcodes, the sync peek, the
  *        layout editor's matrix - words them identically.
  */
  static QString FormatSyncOffset(QmitkMxNSyncDimension dimension, const SyncOffset& offset);

  /** \brief Sorted names of all groups any live cell links for the dimension. */
  std::vector<std::string> GetSyncGroupNames(QmitkMxNSyncDimension dimension) const;

  /**
  * \brief Re-establish `reference + offset` for every member of the group on
  *        the given dimension, where the reference is the live state of the
  *        group's pre-order first member (the seed).
  *
  *   The safety net for delta drift: boundary clamping, missed events, and
  *   the pan-offset perturbation under zoom all desync members from their
  *   declared offsets; re-converging restores them.
  *
  * \throws mitk::Exception if the dimension carries no convergence
  *         bookkeeping (only `Slice`, `Zoom`, and `Pan` do), or if no cell
  *         links the group for this dimension.
  */
  void ReconvergeSyncGroup(QmitkMxNSyncDimension dimension, const std::string& group);

  /**
  * \brief Set a cell's view direction and relay it to the cell's
  *        `Orientation` group (if linked).
  *
  *   Members receive the plane through the silent programmatic path, so a
  *   relayed change never re-triggers propagation. After the relay, the
  *   geometry-relative offsets of the affected cells' `Slice` / `Zoom` /
  *   `Pan` groups are re-converged (the plane flip re-initializes each
  *   member's stepper and camera).
  *
  * \throws mitk::Exception on an unknown window or a plane other than
  *         Axial / Coronal / Sagittal (interactive orientation sync targets
  *         the standard anatomical planes).
  */
  void SetViewDirection(const QString& windowId, mitk::AnatomicalPlane viewDirection);

  /**
  * \brief Set a node's lookup table as a renderer-specific property on the
  *        cell and on every member of the cell's `Lut` group.
  *
  *   Members share the given lookup table instance as their
  *   renderer-specific "LookupTable" property; renderers outside the group
  *   (and the node-global property) stay untouched. The mapper prefers the
  *   renderer-specific property, so grouped cells detach from node-global
  *   colormap changes by design. An unlinked cell gets only its own
  *   renderer's property set.
  *
  * \throws mitk::Exception on an unknown window, a null node, or a null
  *         lookup table.
  */
  void SetLookupTable(const QString& windowId, mitk::DataNode* node, mitk::LookupTable* lookupTable);

  /**
  * \brief Set a node's level/window by value on the cell and on every member
  *        of the cell's `Windowing` group.
  *
  *   Grouped cells receive the value as their renderer-specific "levelwindow"
  *   property (all members end up on the same absolute value); renderers
  *   outside the group and the node-global property stay untouched. An
  *   unlinked cell falls back to the classic node-global write, keeping it
  *   coupled to the global level/window controls.
  *
  *   This is the absolute-set companion of the gesture-driven synchronized
  *   level-window path; use 'AdjustLevelWindow' for deltas that must preserve
  *   the members' relative differences.
  *
  * \throws mitk::Exception on an unknown window or a null node.
  */
  void SetLevelWindow(const QString& windowId, mitk::DataNode* node, const mitk::LevelWindow& levelWindow);

  /**
  * \brief Apply a level/window delta on the cell and on every member of the
  *        cell's `Windowing` group.
  *
  *   Mirrors the synchronized gesture semantics: the delta is applied to each
  *   member's own current value (renderer-specific, falling back to
  *   node-global) and written renderer-specific, so members keep their
  *   relative differences. An unlinked cell falls back to the classic
  *   node-global write.
  *
  * \throws mitk::Exception on an unknown window or a null node.
  */
  void AdjustLevelWindow(const QString& windowId, mitk::DataNode* node,
                         mitk::ScalarType levelDelta, mitk::ScalarType windowDelta);

  /**
  * \brief The hue that identifies the group across all furniture surfaces.
  *
  *   Assigned from a fixed palette by first-registration order (creation of a
  *   selection group or first link of a navigation/windowing/lut group), so
  *   the assignment is stable for the lifetime of the current layout and
  *   identical for every cell. The palette wraps when more groups exist than
  *   palette entries.
  *
  * \throws mitk::Exception on a group that was never registered.
  */
  QColor GetSyncGroupColor(const std::string& group) const;

  /**
  * \brief The window-perspective sync-barcode slots for one cell: eight axes in
  *        fixed order (the seven 'QmitkMxNSyncDimension' axes, then data
  *        selection), each carrying the group hue when the cell is linked on
  *        that axis and an invalid color (a gap) otherwise, plus the axis glyph
  *        and a per-slot tooltip.
  *
  *   Binary per axis - a cell is linked or not; the heterogeneous group state
  *   is a group-perspective concern, not a cell's. Every cell always carries a
  *   selection group (the default group), and a fresh cell also links Windowing
  *   and LUT to the default group, so a cell at rest paints three slots in its
  *   hue (Windowing, LUT, selection), not gaps. Shared by the per-cell utility-strip barcode and
  *   the sync peek plates so both surfaces tell the same story. Returns
  *   eight gap slots for an unknown cell.
  */
  QList<QmitkMxNSyncBarcodeWidget::AxisSlot> BuildBarcodeSlots(const QString& windowId) const;

  /**
  * \brief A cell's name for display: its layout-document display name, or, for
  *        a layout that never named it, the bare tail of its id after the
  *        editor prefix.
  */
  QString CellLabel(const QString& windowId) const;

  /**
  * \brief Raise or lower the sync peek in every visible cell that can host it,
  *        emphasising 'axis' - or none of them without one, which is what the
  *        pointer resting on a barcode between two glyphs shows.
  *        Applies immediately, without the pointer dwell the hover path uses.
  *        Public so the peek is testable without a pointer.
  *
  *   The state is held even in a layout where no cell can host a plate (nothing
  *   is shown then, which is a documented limit), so the gesture reads the same
  *   whatever the geometry.
  */
  void SetSyncPeek(bool visible, std::optional<QmitkMxNSyncAxis> axis);

  /** \brief Whether the hover peek is up. Arrange mode keeps the plates up on
   *         its own and does not count here. */
  bool IsSyncPeekVisible() const;

  /** \brief The axis the sync peek emphasises, if any. */
  std::optional<QmitkMxNSyncAxis> GetSyncPeekAxis() const;

  /**
  * \brief Override the pointer dwell and the teardown grace, in milliseconds.
  *        Defaults are 250 and 150. Exposed so a test can collapse both to zero
  *        and drive the gesture deterministically instead of against the clock.
  */
  void SetSyncPeekTimings(int dwellMs, int graceMs);

  /** \brief The one plate geometry every cell of a layout shares. */
  struct PeekGeometry
  {
    int glyphBox = 0;  // 0 when no visible cell can host a plate
    QmitkMxNPeekRows rows = QmitkMxNPeekRows::One;
  };

  /**
  * \brief The plate geometry of this layout. For each glyph arrangement, the
  *        box is the smallest desirable one across the visible cells that can
  *        host a plate in it, clamped to the legible range. The arrangement
  *        that gives more cells a plate wins, then the one with the larger
  *        box, then the single row. A zero box, when no cell can host a plate
  *        either way, is how the peek stays down in a layout of slivers.
  *
  *   A cell too small for a plate shows none rather than dragging every other
  *   window down to its size: the comparison survives a missing member better
  *   than it survives eight different geometries. But a missing plate is a
  *   window arrange mode cannot select, so fitting more plates beats a larger
  *   one.
  */
  PeekGeometry ResolvePeekGeometry() const;

  /**
  * \brief Pointer report from one cell's barcode: starts the dwell when the peek
  *        is down, changes the emphasised axis immediately when it is up, and
  *        starts the teardown grace once the pointer is off the strip.
  *
  *   The peek's lifetime is the pointer's stay on the barcode, not on one glyph:
  *   the strip is a single surface, and the answer must survive the gaps between
  *   its glyphs and its inert trailing space. The emphasis latches for the same
  *   reason - it moves when another glyph claims it and is dropped when the
  *   pointer leaves, never by the gap in between.
  */
  void OnSyncPeekHovered(bool overStrip, std::optional<QmitkMxNSyncAxis> axis);

  /**
  * \brief This editor's arrange mode. While it is on, every visible cell that
  *        can host a peek plate shows one, whatever the hover peek does: a
  *        plate is up while arrange mode is on OR the hover peek is up, and
  *        lowering the hover peek clears only its own term.
  */
  QmitkMxNArrangeMode* GetArrangeMode() const;

  /**
  * \brief Re-resolve the shared glyph box and re-raise the plates on the next
  *        event-loop pass. Cells call this when they resize; requests within
  *        one pass coalesce into one refresh.
  */
  void RequestSyncPeekRefresh();

  /**
  * \brief The window ids that share one synchronization: the members of
  *        'group' on 'axis'. Empty for an unknown group, an axis the group
  *        links for no cell, or a transient mid-layout-change state.
  */
  QStringList CellsSharingAxis(const QString& group, QmitkMxNSyncAxis axis) const;

  /**
  * \brief The group 'windowId' is on for 'axis' - the per-dimension link, or
  *        the selection connector for the selection axis; empty when the cell
  *        synchronizes nothing there.
  */
  std::string ResolveCellAxisGroup(const QString& windowId, QmitkMxNSyncAxis axis) const;

  /** \brief Which single group identity, if any, to paint on a cell's frame. */
  enum class CellGroupIdentityKind
  {
    None,     // the cell has no group on any of the eight axes (not normally reachable)
    Mono,     // every axis the cell is tied on names one group
    Complex   // the cell's tied axes span more than one group
  };

  struct CellGroupIdentity
  {
    CellGroupIdentityKind kind = CellGroupIdentityKind::None;
    QColor hue;  // valid only when kind == Mono
  };

  /**
  * \brief Resolve a cell's frame identity from its navigation/intensity
  *        membership.
  *
  *   Resolves over all eight axes - the seven 'QmitkMxNSyncDimension' links plus
  *   the cell's data-selection group. 'Mono' when they all name one group (the
  *   hue is that group's color); 'Complex' (gray) when they span more than one;
  *   'None' only when the cell has no group at all (not reachable in normal
  *   operation, since every cell has a selection group). A fresh cell is
  *   'Mono' in the default group - it links Windowing, LUT, and selection to
  *   it. A cell
  *   whose navigation/intensity axes name one group while its selection stays on
  *   another (or vice versa) is an honest split and reads 'Complex'; there is no
  *   "navigation wins" tiebreak. A group color that throws mid-layout-change
  *   downgrades the result to 'None' for that pass. Consumed by
  *   'RefreshFrameColors'.
  */
  CellGroupIdentity ResolveCellGroupIdentity(const QString& windowId) const;

  /**
  * \brief Read-only description of one synchronization group, for the sync
  *        editor and other furniture surfaces to render.
  */
  struct SyncGroupInfo
  {
    std::string id;           // URL-safe group id (the `groups` dict key)
    std::string displayName;  // groups.<id>.name; equals the id when unset
    QColor color;             // explicit groups.<id>.color, or the default hue
    bool hasExplicitColor = false;
    std::vector<QString> selectionMembers;  // cells whose links.selection names this group
    std::map<QmitkMxNSyncDimension, std::vector<QString>> members;  // per-dimension membership
  };

  /**
  * \brief The editor's current groups: every registered selection group plus
  *        every group a live cell links on any dimension, in stable
  *        first-registration order. Member lists are in cell pre-order.
  */
  std::vector<SyncGroupInfo> GetSyncGroupInfos() const;

  /**
  * \brief The group's display name (`groups.<id>.name`), falling back to the
  *        id when none is set. The id itself never changes in-app; the
  *        display name carries all human-facing identity.
  */
  std::string GetSyncGroupDisplayName(const std::string& id) const;

  /** \brief Display name of the selection group with the given engine index. */
  QString GetSyncGroupDisplayName(GroupSyncIndexType index) const;

  /**
  * \brief The group id of the selection group with the given engine index.
  *
  * \throws mitk::Exception on an unregistered index.
  */
  std::string GetSyncGroupName(GroupSyncIndexType index) const;

  /**
  * \brief The id of the default selection group (engine index 1), which
  *        every fresh cell joins and every unlinked selection reverts to.
  *
  * \throws mitk::Exception while no group is registered at index 1.
  */
  std::string GetDefaultSyncGroupName() const;

  /**
  * \brief Set the group's display name (cosmetic write to `groups.<id>.name`
  *        only; links, engine indices, and the id itself stay untouched).
  *        An empty name reverts the display name to the id.
  *
  * \throws mitk::Exception on a group that was never registered.
  */
  void SetSyncGroupDisplayName(const std::string& id, const std::string& displayName);

  /**
  * \brief Set the group's hue (cosmetic write to `groups.<id>.color` only).
  *        The color is persisted with the layout and honored verbatim.
  *
  * \throws mitk::Exception on a group that was never registered or an
  *         invalid color.
  */
  void SetSyncGroupColor(const std::string& id, const QColor& color);

  /**
  * \brief Clean-view mode: suppress all viewport furniture in every cell
  *        (readouts, ribbons, proximity reveals), e.g. for taking clean
  *        screenshots with external tools. Sticky until switched off;
  *        applies to cells created later as well. Emits 'CleanViewChanged'
  *        so per-cell toggles can mirror the state.
  */
  void SetCleanView(bool cleanView);
  bool IsCleanView() const;

  /**
  * \brief The key that toggles clean view while focus is inside the editor;
  *        shared with the menus and tooltips that advertise it.
  */
  static QKeySequence CleanViewShortcut();

  /**
  * \brief Default visibility of the per-cell level/window corner readout
  *        (preference-controlled). When off, the readout follows the
  *        proximity reveal instead of being always-on.
  */
  void SetLevelWindowReadoutVisible(bool visible);

  /**
  * \brief Per-cell navigator mode, mirrored editor-wide: compact (the slice
  *        slider only, the frequent action) or expanded (the full 3D
  *        crosshair as three axis sliders plus coordinate entry). "Show
  *        everything" is an acute working mode, not per-window state, so the
  *        live toggle is editor-wide like clean-view; the persisted default
  *        comes from the MxN preference. Applies to cells created later as
  *        well and emits 'NavigatorExpandedChanged' so per-cell affordances
  *        can mirror the state.
  */
  void SetNavigatorExpanded(bool expanded);
  bool IsNavigatorExpanded() const;

  /**
  * \brief Give one cell the whole editor area, hiding its siblings; an empty
  *        or unknown id restores the grid.
  *
  *        Purely transient view state, deliberately implemented by toggling
  *        widget visibility: the splitter structure is untouched, so a
  *        maximized cell is invisible to 'SerializeLayout' and cannot be
  *        persisted. A layout change restores the grid first, because the cell
  *        set it was maximizing out of no longer exists afterwards.
  */
  void SetMaximizedCell(const QString& windowId);

  /** \brief The maximized cell's window id, empty when the grid is shown. */
  QString GetMaximizedCell() const;

  /**
  * \brief Each cell's rectangle within the editor, as a fraction of the whole,
  *        keyed by window id.
  *
  *        Derived from the splitter proportions rather than from on-screen
  *        geometry: the sizes are set as the tree is built, so these are right
  *        before Qt's layout pass has run, where widget geometry would still be
  *        stale. While a cell is maximized the proportions captured on the way
  *        in are used, so the map describes the grid rather than the one
  *        visible cell - the same source 'SerializeLayout' reports.
  */
  std::vector<std::pair<QString, QRectF>> GetNormalizedCellRects() const;

  /**
  * \brief Raise a modal "loading layout" dialog, or take it down.
  *
  *   A dialog rather than an overlay widget, deliberately. The editor's cells
  *   are 'QVTKOpenGLNativeWidget's, and a raster widget stacked above nested
  *   render-to-texture widgets is not composited on top of them - an overlay
  *   covering the editor is painted and simply never seen. A dialog is a window
  *   of its own, with no compositing relationship to the editor's cells.
  *
  *   The caller must raise it, return to the event loop once so it is presented,
  *   and only then apply the layout: 'ApplyLayout' blocks the UI thread, and a
  *   thread that pumps no messages gets nothing composited, so anything raised
  *   after the rebuild starts stays invisible until it ends. Nothing is shown
  *   for a hidden editor.
  *
  *   The dialog is modal and the pumping excludes user input, but posted
  *   events and queued cross-thread calls are still delivered while it is up.
  *   'ApplyLayout' is guarded against being re-entered from there.
  */
  void ShowLayoutLoadFeedback();
  void HideLayoutLoadFeedback();

  /** \brief How a layout-editor request treats the layout editor: Toggle
   *         hides a visible one and shows a hidden one, Show only ever brings it
   *         up, Hide only ever takes it down. */
  enum class LayoutEditorRequest
  {
    Toggle,
    Show,
    Hide
  };

  /**
  * \brief Ask the hosting layer for the layout editor (emits
  *        'LayoutEditorRequested'). Entry point for furniture that cannot
  *        emit the editor's signal itself (e.g. a cell's context menu).
  */
  void RequestLayoutEditor(LayoutEditorRequest request);

  /**
  * \brief Re-initialize the geometry of the cell's geometry-authority
  *        component: every cell reachable from `windowId` over shared
  *        `Slice` or `Orientation` groups (the connected component of the
  *        slice/orientation link graph).
  *
  *   All component members are initialized to one shared geometry - the
  *   bounding geometry of the data storage's nodes as visible in the
  *   triggering cell ("last reinit wins") - via per-window initialization;
  *   cells outside the component (and other editors) are untouched, unlike
  *   the application-global Data Manager reinit. That reinit is left global:
  *   it is the rendering manager's path shared by every editor, so scoping it
  *   would change all of them; this call is the MxN-local way back to
  *   per-group geometry after it. Afterwards the component's
  *   geometry-relative offsets (`Slice` / `Zoom` / `Pan` groups touching
  *   the component) are re-converged.
  *
  * \throws mitk::Exception on an unknown window or when no data storage is set.
  */
  void ReinitSyncGroupGeometry(const QString& windowId);

  /**
  * \brief Construct a render-window widget with a caller-supplied id.
  *
  *   The id is the canonical, fully-qualified window name in the form
  *   `<multiWidgetName>__<bareSegment>`. It is registered with
  *   `RenderingManager` verbatim and is the same string that appears in the
  *   v2 layout document's per-window 'id' field, in REST URLs, and in
  *   per-renderer DataNode property context keys. The editor neither
  *   prepends nor strips a prefix.
  *
  *   This is the canonical creation API; callers that need a deterministic
  *   id (e.g. the layout applier) should go through it. Internal positional
  *   creation (used by 'SetLayout(r, c)') uses a private nullary overload that
  *   delegates here with a collision-free `<multiWidgetName>__widget<i>` id.
  *
  * \param id  The fully-qualified window id (e.g. "mxn__widget0",
  *            "mxn__alpha"). Must be non-empty, must start with
  *            `<multiWidgetName>__`, and must not collide with an existing
  *            render-window in this editor.
  *
  * \return  Shared pointer to the newly constructed render-window widget.
  *
  * \pre  id is non-empty                                  (otherwise mitk::Exception)
  * \pre  id starts with `<multiWidgetName>__`             (otherwise mitk::Exception)
  * \pre  no existing render-window uses the same id       (otherwise mitk::Exception)
  *
  * \throws mitk::Exception on precondition violation.
  */
  RenderWindowWidgetPointer CreateRenderWindowWidget(const QString& id);

  /**
  * \brief Serialize the current layout tree to a v3.0 JSON document
  *        (always strict mode).
  *
  *   Group naming convention: engine-internal sync-group index 1 maps to the
  *   bare label "main"; other indices map to `g_<i>` where `<i>` is a counter
  *   assigned by pre-order encounter order over the cell list. Same engine
  *   state in produces the same group names out (round-trip stable).
  *
  *   Per-cell synchronization links are emitted for every linked dimension;
  *   `slice` / `zoom` / `pan` links carrying a non-identity offset use the
  *   object form (`{"target": ..., "offset": ...}`), all other links the
  *   string shorthand. Every referenced group is declared in the top-level
  *   `groups` dict (groups referenced only by navigation dimensions as empty
  *   entries - they carry no persisted per-group state).
  *
  *   See 'mxn-layout-v3.schema.json' for the document shape this method emits.
  *
  * \pre  Must be called on the UI thread.
  * \pre  The root layout contains exactly one QSplitter (canonical post-load
  *       shape).
  *
  * \throws mitk::Exception if the layout-tree invariant is violated.
  */
  nlohmann::json SerializeLayout() const;

  /**
  * \brief Plain-data summary of one cell leaf in the layout tree.
  *
  *   Holds the per-cell fields that the v2 layout document persists for a
  *   window -- identity (id), optional display label, view direction enum
  *   value, and selection-group label -- without dragging the JSON or
  *   QSplitter shape across the API boundary. Future v3 dimensions add
  *   fields here additively.
  *
  *   Identity vs. display label: 'id' is the v2 schema's required `id`
  *   field -- the fully-qualified, URL-segment-safe canonical window name
  *   (`<multiWidgetName>__<bareSegment>`), unique within the document, used
  *   verbatim as the engine-side render-window name and as the URL path
  *   segment for REST sub-resources. 'displayName' is the optional `name`
  *   field -- a free-form human-readable label, empty when absent.
  */
  struct WindowDescriptor
  {
    QString id;              // canonical fully-qualified window id
    QString displayName;     // optional human-readable label, empty when absent
    QString viewDirection;   // "axial" | "sagittal" | "coronal" | "original"
    QString selectionGroup;  // links.selection group label
  };

  /**
  * \brief List all cell leaves in the current layout tree, in pre-order
  *        traversal order.
  *
  *   This is the engine query that the REST bridge layer (and any other
  *   consumer that needs to know which windows the editor currently has)
  *   should use. Returns plain structs - no JSON, no Qt widget pointers.
  *   The returned descriptors carry the canonical fully-qualified window
  *   id (`<multiWidgetName>__<bareSegment>`), the same string the bridge
  *   layer receives from REST URLs.
  *
  *   `SerializeLayout` shares the per-cell descriptor logic via
  *   `MakeWindowDescriptor` but performs its own splitter-tree walk to
  *   emit topology + sizes; both walks therefore agree on per-cell
  *   field values by construction.
  *
  * \pre  Must be called on the UI thread.
  * \pre  The root layout contains exactly one QSplitter (canonical
  *       post-load shape).
  *
  * \throws mitk::Exception if the layout-tree invariant is violated, or
  *         if a cell references a sync-group index with no entry in the
  *         engine's group-name registry.
  */
  std::vector<WindowDescriptor> ListWindowDescriptors() const;

  /**
  * \brief Apply a v2.0 or v3.0 JSON document.
  *
  *   Tears down all existing render windows and rebuilds from scratch (no
  *   positional reuse). On failure during construction, rolls back to a
  *   single default cell and rethrows.
  *
  *   See 'mxn-layout-v3.schema.json' for the accepted document shape. A
  *   v2.0 document is the compatible subset: missing `links` keys default to
  *   unsynced per-cell singletons, and unknown link keys stay tolerated
  *   (silently ignored) so existing v2 files load unchanged. For a v3.0
  *   document the `links` object is closed: an unknown link key, an unknown
  *   modifier, or an `offset` on a dimension that does not accept it (or of
  *   the wrong type) is rejected.
  *
  *   Id contract: every window's `id` MUST already be in the canonical
  *   fully-qualified form `<multiWidgetName>__<bareSegment>` matching this
  *   editor's `multiWidgetName`. The loader does NOT prepend or strip a
  *   prefix; what the document holds is what the engine uses. Documents
  *   written for a different editor instance are rejected up-front with a
  *   message naming the offending id.
  *
  *   Group seeding: after the new cell tree is built, each group's runtime
  *   synchronized state (per-renderer 'visible' / 'layer') is seeded from
  *   the cell that appears first in document order whose links.selection
  *   names that group; remaining members are normalised to the seed. See
  *   the canonical rule on the schema's `groups` description.
  *
  *   Not re-entrant. The rebuild pumps the event loop to keep its progress
  *   dialog moving, so posted events and queued calls can run in the middle of
  *   it; a call made from there, while this one is still in progress, is
  *   rejected with 'QmitkMxNLayoutBusyException' and leaves the running apply
  *   untouched. See 'IsApplyingLayout'.
  *
  * \param doc  A parsed v2.0 or v3.0 layout document.
  *
  * \pre  Must be called on the UI thread.
  *
  * \throws QmitkMxNLayoutBusyException if another 'ApplyLayout' on this editor
  *         is still in progress.
  * \throws mitk::Exception on: version neither "2.0" nor "3.0"; structural
  *         shape violation; id not starting with `<multiWidgetName>__`;
  *         duplicate window ids; unknown view_direction; missing group
  *         reference in strict mode; v3 links-closure violation (unknown
  *         link key / modifier, misplaced or mistyped offset);
  *         nlohmann parse / type errors (rewrapped from
  *         'nlohmann::json::exception' subtypes).
  */
  void ApplyLayout(const nlohmann::json& doc);

  /**
  * \brief True while an 'ApplyLayout' is in progress, including its rollback on
  *        failure.
  *
  *   The cell tree is torn down and half rebuilt during that time. Code that
  *   can run from the event loop the rebuild pumps (posted events, queued
  *   cross-thread calls) must check this and leave the editor alone while it
  *   is true.
  */
  bool IsApplyingLayout() const;

  /**
  * \brief True (and 'rows' / 'columns' filled) when the current layout is a
  *        rectangular grid.
  *
  *   A rectangular grid is: the root splitter is vertical, every child is a
  *   horizontal splitter, each holds only 'QmitkRenderWindowWidget' cells, and
  *   all rows have the same non-zero cell count. Derived from the actual
  *   splitter tree, so it is correct even when the stored 'GetRowCount()' is 0
  *   (a loaded layout) or stale (after a render-window layout-design-menu
  *   change, which rebuilds the tree without touching the counts). The grid-op
  *   guards and the layout editor's grid buttons read this rather than the
  *   stored counts.
  */
  bool ResolveGridShape(int& rows, int& columns) const;

  /**
  * \brief Append one fresh, empty cell to the right of every row of a
  *        rectangular grid (r x c -> r x (c+1)).
  *
  *   Existing cells keep their widgets, ids, sync links, node selection, and
  *   tree positions untouched; only the trailing column is new. New cells come
  *   from the nullary 'CreateRenderWindowWidget()' (collision-free id, default
  *   sync group 1, all data shown), then are added to the right of each row.
  *   Cell ids are no longer row-major after this call (they carry uniqueness
  *   only); the splitter tree, not the id, is the authority on cell position.
  *   No-op (with a 'MITK_WARN') when the current layout is not a rectangular
  *   grid.
  */
  void AddGridColumn();

  /**
  * \brief Remove the rightmost cell of every row of a rectangular grid
  *        (requires >= 2 columns; otherwise a no-op).
  *
  *   Survivors are untouched. Each removed cell is torn down in the order the
  *   surrounding ownership rules require: its selection-group index is captured
  *   before removal (removal can destroy the cell and its utility widget, so
  *   reading the index afterwards would be use-after-free), the active render
  *   window is repointed to a surviving cell first if it is among those removed
  *   (the active pointer is a strong reference, so an unreset active cell would
  *   outlive the map erase and linger as a ghost), then the cell is removed, its
  *   'm_CellSyncLinks' entry erased, and its former selection group reclaimed if
  *   it is now empty.
  */
  void RemoveGridColumn();

  /**
  * \brief Append a fresh, empty bottom row (a new horizontal row-split of
  *        'columnCount' fresh cells). Existing cells untouched. No-op (with a
  *        'MITK_WARN') when the current layout is not a rectangular grid.
  */
  void AddGridRow();

  /**
  * \brief Remove the bottom row of a rectangular grid (requires >= 2 rows;
  *        otherwise a no-op) and its cells (same per-cell teardown as
  *        'RemoveGridColumn'), then delete the emptied bottom row-splitter -
  *        destroying a row's cells does not delete their parent 'QSplitter', and
  *        a leftover childless split node would break 'ResolveGridShape' and the
  *        'SerializeLayout' tree walk.
  */
  void RemoveGridRow();

public Q_SLOTS:

  // mouse events
  void wheelEvent(QWheelEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void moveEvent(QMoveEvent* e) override;

  /**
  * \brief Slot wrapper around 'ApplyLayout'. Loads a v2.0 or v3.0 layout
  *        document (replaces the current cell tree).
  *
  * \param jsonData  Pointer to a parsed layout document. Must not be null
  *                  and must not represent a JSON null value.
  *
  * \pre   jsonData != nullptr                            (otherwise mitk::Exception)
  * \pre   !jsonData->is_null()                           (otherwise mitk::Exception)
  *
  * \throws mitk::Exception (rethrown from 'ApplyLayout') on null pointer,
  *         JSON null value, or any of the 'ApplyLayout' failure conditions.
  */
  void LoadLayout(const nlohmann::json* jsonData);

  /**
  * \brief Slot wrapper around 'SerializeLayout'. Writes the current layout
  *        as a v3.0 JSON document (pretty-printed) to 'outStream'.
  *
  *   No-op if 'outStream' is null. Otherwise emits the JSON returned by
  *   'SerializeLayout' followed by a newline.
  *
  * \param outStream  Output stream. May be null (no-op).
  *
  * \throws mitk::Exception (rethrown from 'SerializeLayout') if the layout
  *         tree is in an invariant-violating state (e.g. no top-level
  *         splitter).
  */
  void SaveLayout(std::ostream* outStream);

  void SetDataBasedLayout(const QmitkAbstractNodeSelectionWidget::NodeList& nodes);

  /**
  * \brief Create a new data-selection group and assign the given node selection
  *        widget to it: allocates the next free group index via
  *        'NextFreeSyncGroupIndex', creates the group, and assigns the widget.
  */
  void OnCreateNewSyncGroupRequested(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget);

Q_SIGNALS:

  void WheelMoved(QWheelEvent *);
  void Moved();
  void LayoutChanged();
  void SyncGroupAdded(const GroupSyncIndexType index, const QString& label);
  void CleanViewChanged(bool cleanView);
  void NavigatorExpandedChanged(bool expanded);

  /** \brief The maximized cell changed; empty id means the grid is back. */
  void MaximizedCellChanged(const QString& windowId);

  /** \brief Editor-wide crosshair visibility changed, so per-cell affordances
   *         can mirror it. */
  void CrosshairVisibilityChanged(bool visible);

  /**
  * \brief A divider was dragged, so the cells still are what they were but no
  *        longer where. Surfaces the splitters' own 'splitterMoved' without
  *        exposing the tree, for anything mirroring the layout's proportions.
  */
  void LayoutProportionsChanged();

  /**
  * \brief A selection group's display label changed (cosmetic rename);
  *        per-cell group selectors update their row text.
  */
  void SyncGroupLabelChanged(const GroupSyncIndexType index, const QString& label);

  /**
  * \brief Something about the per-dimension links or the group cosmetics
  *        changed; structural furniture (layout editor, barcodes, frames,
  *        plates) re-reads the engine state.
  */
  void SyncLinksChanged();

  /**
  * \brief A cell asked for the layout editor: its sync barcode toggles the
  *        view, its context menu shows it. The hosting layer (the BlueBerry
  *        editor part) acts on the request; the module only relays it.
  */
  void LayoutEditorRequested(QmitkMxNMultiWidget::LayoutEditorRequest request);

protected:

  /**
  * \brief Look up the connector backing the synchronization group with the
  *        given index. Returns nullptr if no group with that index exists.
  *
  *   Internal accessor that all in-class code goes through (rather than
  *   reaching into 'm_SynchronizedWidgetConnectors' directly), keeping the
  *   map encapsulated. Exposed as 'protected' so a test-only subclass can
  *   surface it for white-box assertions on connector identity / state
  *   preservation; production code outside the class hierarchy must not
  *   depend on this.
  */
  QmitkSynchronizedWidgetConnector* GetSyncGroupConnector(const GroupSyncIndexType index) const;

  /** \brief Number of currently registered synchronization groups. */
  std::size_t GetSyncGroupCount() const;

  /**
  * \brief Number of member applications performed by orientation
  *        propagation since construction.
  *
  *   Test seam (surfaced by a test-only subclass, like
  *   'GetSyncGroupConnector'): one plane change relayed to N group members
  *   increments this by exactly N; a propagation cycle would inflate it.
  */
  unsigned int GetOrientationApplyCount() const;

private:

  void SetLayoutImpl() override;

  /**
  * \brief Pre-mutation check that every window id in `doc` belongs to this
  *        editor instance.
  *
  *   Walks every `window` node in the layout tree and rejects ids that do
  *   not start with `<multiWidgetName>__`. Throws with a message naming the
  *   offending id and the expected prefix. Run as the first step of
  *   `ApplyLayout`, before any engine state is touched, so a misrouted
  *   document does not destroy the existing layout on the way out.
  *
  *   The schema's pattern enforces structural shape (id must contain `__`)
  *   but cannot encode "matches THIS editor's `multiWidgetName`": that
  *   constraint is loader-instance-specific and lives here.
  */
  void ValidateIdsForThisEditor(const nlohmann::json& doc) const;

  /**
  * \brief Positional convenience overload used by 'SetLayout(r, c)',
  *        'InitializeMultiWidget', and 'SetDataBasedLayout'.
  *
  *   Picks the smallest non-negative `i` such that
  *   `<multiWidgetName>__widget<i>` is not already used as an id in this
  *   editor, then delegates to the explicit-id overload. This replaces the
  *   old `widget<count>` form, which silently collided when custom-id'd
  *   cells already used the same index.
  */
  QmitkAbstractMultiWidget::RenderWindowWidgetPointer CreateRenderWindowWidget();

  QmitkAbstractMultiWidget::RenderWindowWidgetPointer GetWindowFromIndex(size_t index);

  /**
  * \brief The root vertical splitter of the cell tree
  *        ('layout()->itemAt(0)->widget()' as a 'QSplitter'), or nullptr when
  *        the editor has no splitter-based layout. The single access point the
  *        grid ops and 'ResolveGridShape' share with 'SerializeLayout' /
  *        'ListWindowDescriptors'.
  */
  QSplitter* RootSplitter() const;

  /**
  * \brief Relay every layout splitter's 'splitterMoved' to
  *        'LayoutProportionsChanged'.
  *
  *        Splitters are created in a dozen places, several of them in the
  *        shared layout manager, so the tree is walked after a structural
  *        change instead of wiring each construction site. The connection is
  *        unique, which makes re-walking idempotent, and the walk is by
  *        child index rather than findChildren so splitters that belong to a
  *        render window rather than the layout stay out of it.
  */
  void RelaySplitterProportionChanges();

  /**
  * \brief Tear down a single cell during grid surgery: capture its selection
  *        group index (before removal, see below), remove it from the
  *        render-window registry, erase its 'm_CellSyncLinks' entry, and reclaim
  *        its former selection group if that group is now empty. The index must
  *        be read before 'RemoveRenderWindowWidget' because that call can drop
  *        the cell's last owning reference and destroy it (and its utility
  *        widget). Does not repoint the active render window - the caller does
  *        that once, before removing any cell, so it can pick a survivor.
  */
  void DetachAndDestroyCell(QmitkRenderWindowWidget* cell);

  /**
  * \brief If the active render window is among 'removalSet', repoint it to the
  *        surviving top-left cell (tree row 0, column 0) before any removal.
  *        Neither trailing-edge removal touches that cell, and the active
  *        pointer is a strong reference: leaving it on a doomed cell would keep
  *        that cell alive and visible after its registry entry is erased.
  */
  void ResetActiveIfRemoved(const std::vector<QmitkRenderWindowWidget*>& removalSet);

  /**
  * \brief Common tail of the four grid ops: re-derive the grid shape from the
  *        mutated tree, store it via 'SetGridDimensions' (keeping the
  *        rows*columns == cell-count invariant), and emit 'LayoutChanged' so the
  *        group cards, the plates and other furniture refresh. Counts are set
  *        before the signal so no listener observes a counts-vs-tree mismatch.
  */
  void FinalizeGridSurgery();

  /**
  * \brief Recursive serializer for a 'split' subtree. Emits a v3 JSON node.
  *
  *   The root's 'size' field is omitted not by a flag but structurally:
  *   the parent loop attaches 'size' to each child before pushing into
  *   the children array; the root, having no parent loop, never gets one.
  *
  *   The 'groupNames' map provides the bare group name for each engine-
  *   internal sync-group index encountered in the cell tree. It must be
  *   pre-populated by the caller before recursing.
  */
  nlohmann::json SerializeSplitter(const QSplitter* splitter,
                                   const std::map<GroupSyncIndexType, std::string>& groupNames) const;

  /**
  * \brief Build a 'WindowDescriptor' for a single cell.
  *
  *   Centralizes the per-cell field lookup: id from the cell's render-window
  *   name (used verbatim, no prefix translation), display name from the
  *   cell's own state, view-direction from the slice-navigation controller's
  *   default direction, selection group label from the engine's group-name
  *   registry. Both 'ListWindowDescriptors' and 'SerializeSplitter' use this
  *   so the per-cell descriptor logic lives in one place.
  *
  * \pre  cell != nullptr
  * \pre  cell's sync-group index has an entry in 'm_GroupNameByIndex'
  *
  * \throws mitk::Exception on precondition violation.
  */
  WindowDescriptor MakeWindowDescriptor(const QmitkRenderWindowWidget* cell) const;

  /**
  * \brief Recursive constructor for a 'split' subtree. Returns a freshly
  *        allocated QSplitter with the cell tree below.
  *
  *   Window leaves are created via 'CreateRenderWindowWidget(id)' using the
  *   document's id verbatim, then re-parented to the new splitter and moved
  *   into their target sync group via 'SetSynchronizationGroup'.
  */
  QSplitter* BuildSplitterFromJson(const nlohmann::json& splitNode,
                                   const std::map<std::string, GroupSyncIndexType>& nameToInt,
                                   QSplitter* parentSplitter);

  /**
  * \brief Group seeding pass for ApplyLayout.
  *
  *        Runs after the new cell tree has been constructed (so cells are
  *        registered under their qualified names). For each referenced
  *        group, the cell that appears first in document order in
  *        'seedingOrder' becomes that group's seed. Its per-renderer node
  *        properties (visible, layer) are written into the connector via
  *        'SeedFromMember', then propagated to every other member of the
  *        group via 'SynchronizeWidget'. Pre-seeding divergence between the
  *        seed and other members is reported via 'MITK_WARN', capped to a
  *        small budget.
  *
  *        Group-scoped state in the layout document's 'groups' dict
  *        (e.g. 'select_all') is set by the caller before this method runs
  *        and is not touched here.
  *
  * \note  Cost of the propagation pass on a group is O(N_nodes * N_members^2):
  *        for each of the N_members cells, 'SynchronizeWidget' iterates the
  *        selected node list and writes per-cell visibility, which fans
  *        back through the connector to each of the (N_members - 1) other
  *        already-connected widgets. Acceptable for typical layouts (a few
  *        cells, hundreds of nodes); worth re-checking for groups with
  *        many members and very large data storages.
  *
  * \param seedingOrder  Pre-order (windowId, groupName) pairs captured
  *                      during PrewalkValidate.
  * \param nameToInt     Resolved layout group-label to engine-internal
  *                      sync-group index mapping.
  */
  void SeedAndNormalizeGroups(
    const std::vector<std::pair<std::string, std::string>>& seedingOrder,
    const std::map<std::string, GroupSyncIndexType>& nameToInt);

  /**
  * \brief Tear down the current cell set: disconnect all per-cell signals,
  *        drop strong refs (cells self-destruct), delete the layout/splitter
  *        tree, clear sync-group connectors, null the active-widget pointer.
  *
  *   Used by 'ApplyLayout' before construction and by 'RollBackToSingleDefaultCell'.
  */
  void TearDownAllCells();

  /**
  * \brief Restyle every cell's frame from its group identity, and be the single
  *        authoritative MxN writer of the border stylesheet. The border color is
  *        the cell's mono group hue, or a neutral gray when it is ungrouped or
  *        heterogeneous - carried by every cell, active or not, so an active
  *        grouped cell keeps its hue. Active-ness is marked by white corner
  *        brackets the cell overlay paints on top. Connected to
  *        'SyncLinksChanged' and 'LayoutChanged', and invoked on the active-cell
  *        change (which repaints the overlays). Never routes through the shared
  *        'SetDecorationColor' (which would leak to StdMultiWidget).
  */
  void RefreshFrameColors();

  /**
  * \brief The engine index of the selection connector for the string group
  *        'group', allocating (and tracking for later reclaim) one when the
  *        group has no connector yet.
  */
  GroupSyncIndexType EnsureSelectionGroupIndex(const std::string& group);

  /**
  * \brief Deregister a selection connector once its last member has left, but
  *        only if this class allocated it (see 'EnsureSelectionGroupIndex');
  *        document- and default-seed connectors persist even when empty.
  */
  void ReclaimSelectionGroupIfEmpty(GroupSyncIndexType index);

  /**
  * \brief Push each cell's per-dimension group membership to its utility-strip
  *        sync barcode (one hue per linked dimension, a gap otherwise).
  *        Connected to 'LayoutChanged' and 'SyncLinksChanged'.
  */
  void RefreshSyncBarcodes();

  /**
  * \brief Recovery path when 'ApplyLayout' construction fails part-way.
  *        Drains whatever was partially built and re-runs the default
  *        single-cell initialisation so the editor stays in a usable state.
  *        The rolled-back single-cell state has no preset name to claim,
  *        so 'm_LayoutName' is cleared (via 'TearDownAllCells'); a
  *        subsequent 'SerializeLayout' emits no top-level 'name' field.
  */
  void RollBackToSingleDefaultCell();

  /**
  * \brief Install the synchronized display-action handler with one
  *        membership predicate per broadcast navigation dimension.
  *
  *   The predicates read 'm_CellSyncLinks' live at event time, so link
  *   changes take effect without re-wiring. An unlinked cell degenerates to
  *   a singleton: it still receives its own gestures (the synchronized
  *   action is the handler for the sender itself), but nothing propagates.
  */
  void InstallSynchronizedHandler();

  /**
  * \brief Membership predicate: should the synchronized action for
  *        'dimension' propagate from 'sender' to 'target'?
  *
  *   Both renderers must be cells of this editor (qualified-name prefix);
  *   foreign senders and targets are always rejected, which keeps several
  *   editors' broadcasts (each observes every interaction event
  *   application-wide) from double-handling each other's windows.
  */
  bool IsNavTarget(QmitkMxNSyncDimension dimension,
                   const mitk::BaseRenderer* sender,
                   const mitk::BaseRenderer* target) const;

  /**
  * \brief Sender classification for the level-window gesture: foreign for a
  *        renderer that is not a cell of this editor (so that several
  *        editors' broadcasts do not double-handle each other's windows),
  *        ungrouped for a cell without a windowing link (the node-global
  *        write keeps it coupled to the global level/window controls),
  *        grouped otherwise.
  */
  mitk::DisplayActionEventFunctions::LevelWindowScope WindowingScopeOf(const mitk::BaseRenderer* sender) const;

  /**
  * \brief Membership predicate for a grouped sender's level-window gesture:
  *        both renderers are cells of this editor sharing a windowing group.
  */
  bool IsWindowingTarget(const mitk::BaseRenderer* sender, const mitk::BaseRenderer* target) const;

  /**
  * \brief The group's seed cell for a dimension: the pre-order first cell
  *        linking the group. Empty string if no cell links it.
  */
  QString FindSyncGroupSeed(QmitkMxNSyncDimension dimension, const std::string& group) const;

  /**
  * \brief Record 'group' in the hue-assignment order if it is new
  *        (see GetSyncGroupColor).
  */
  void RegisterGroupForHue(const std::string& group);

  /**
  * \brief Remove the renderer-specific 'propertyKeys' of cell 'windowId' from
  *        every node. Cell ids are reused and a renderer-specific value
  *        outranks the node-global one, so a leftover would restyle whichever
  *        cell next takes the id - or keep an unlinked cell detached from the
  *        global controls.
  */
  void RemoveCellRendererProperties(const QString& windowId, std::initializer_list<const char*> propertyKeys);

  /**
  * \brief Shared member loop of 'SetLevelWindow' / 'AdjustLevelWindow':
  *        validate, resolve the cell's `Windowing` group, and run 'modify'
  *        on the level window of every target (renderer-specific for group
  *        members, node-global for an unlinked cell).
  */
  void ApplyLevelWindow(const QString& windowId, mitk::DataNode* node,
                        const std::function<void(mitk::LevelWindow&)>& modify);

  /**
  * \brief Relay a plane change of 'sourceId' to its `Orientation` group and
  *        re-converge the affected geometry-relative offsets.
  *
  *   Members are set through the silent utility-widget path (no signal
  *   re-emission); the depth guard additionally drops any change that
  *   arrives while a relay is in flight, so propagation can never cycle.
  */
  void PropagateOrientation(const QString& sourceId, mitk::AnatomicalPlane viewDirection);

  /**
  * \brief Cells reachable from 'windowId' over shared `Slice` or
  *        `Orientation` groups (the geometry-authority component, always
  *        including 'windowId' itself), in pre-order.
  */
  std::vector<QString> ComputeGeometryComponent(const QString& windowId) const;

  /**
  * \brief Align every component member's reference geometry to the
  *        component seed's (pre-order first member). Members whose geometry
  *        already matches are left alone; without a realized seed geometry
  *        nothing happens (deferred to the next reinit / re-converge).
  *
  * \return The ids of the members that were re-initialized; the caller
  *         re-converges the groups these touch.
  */
  std::vector<QString> EnforceComponentGeometry(const QString& windowId);

  /**
  * \brief Re-converge every `Slice` / `Zoom` / `Pan` group that has at least
  *        one member among 'windowIds' (after their geometry changed).
  */
  void ReconvergeGeometryRelativeGroups(const std::vector<QString>& windowIds);

  /**
  * \brief Absolute-set one member to the seed's live state combined with the
  *        member's declared offset (dimension-typed, see SetSyncLink).
  *
  *   Skips silently while an involved render window has no world geometry
  *   yet (unrealized); re-converge covers the deferred case. The seed
  *   itself is never converged - its live state is the reference.
  */
  void ConvergeMemberToSeed(QmitkMxNSyncDimension dimension, const QString& seedId, const QString& memberId);

  /**
  * \brief Per-cell synchronization links (all dimensions except selection,
  *        which lives in the connector registry). Groups indexed by the
  *        dimension's position in 'QmitkMxNAllSyncDimensions'; the typed
  *        offsets are meaningful only while the matching dimension is
  *        linked.
  */
  struct CellSyncLinks
  {
    std::array<std::optional<std::string>, QmitkMxNAllSyncDimensions.size()> groups;
    int sliceOffset = 0;
    double zoomOffset = 1.0;
    mitk::Vector2D panOffset = mitk::Vector2D(0.0);
  };

  std::map<QString, CellSyncLinks> m_CellSyncLinks;

  /** \brief While active, cells created later auto-join the macro group (see Synchronize). */
  bool m_SynchronizeMacroActive = false;

  /** \brief Re-entrancy guard for orientation propagation (see PropagateOrientation). */
  unsigned int m_OrientationPropagationDepth = 0;

  unsigned int m_OrientationApplyCount = 0;

  std::map < GroupSyncIndexType, std::unique_ptr<QmitkSynchronizedWidgetConnector> > m_SynchronizedWidgetConnectors;

  /**
  * \brief Selection-connector indices this class lazily allocated for a link
  *        group's selection axis (see 'SetCellSelectionGroup'). Only these are
  *        reclaimed when they empty; document- and default-seed connectors are
  *        not tracked here and persist.
  */
  std::set<GroupSyncIndexType> m_SelectionGroupsAllocatedForLinks;

  /**
  * \brief Engine-internal group-name registry keyed by sync-group index.
  *
  *        Group names live nowhere else in memory: the connector map
  *        ('m_SynchronizedWidgetConnectors') is integer-keyed, and layout
  *        documents are not retained after load. This registry is populated
  *        by 'AddSynchronizationGroup' for every group the editor creates
  *        (whether from a layout document, from runtime "+ new group"
  *        actions, or from default initialization), and cleared by
  *        'TearDownAllCells'. 'SerializeLayout' reads from here directly, so
  *        a load -> save round-trip preserves the layout-document labels
  *        (e.g. a doc that uses 'alpha' for engine index 1 round-trips as
  *        'alpha', not the convention default 'main').
  */
  std::map<GroupSyncIndexType, std::string> m_GroupNameByIndex;

  /**
  * \brief Group names in first-registration order; positions index the
  *        default hue palette (see GetSyncGroupColor). Cleared by
  *        'TearDownAllCells' together with the group registry.
  */
  std::vector<std::string> m_GroupHueOrder;

  /**
  * \brief Cosmetic per-group state from the layout document's `groups` dict:
  *        display names (`name`) and hues (`color`, kept as the verbatim hex
  *        string for byte-stable round-trips). Populated by ApplyLayout and
  *        the set-display-name / set-color writes; cleared by
  *        'TearDownAllCells'. Only groups present here emit the fields on
  *        serialization, so documents stay minimal.
  */
  std::map<std::string, std::string> m_GroupDisplayNames;
  std::map<std::string, std::string> m_GroupColors;

  /** \brief Sticky clean-view state; applied to cells created later, too. */
  bool m_CleanView = false;

  /** \brief Window id of the maximized cell, empty while the grid is shown. */
  QString m_MaximizedCell;

  /** \brief Set for the duration of 'ApplyLayout'; see 'IsApplyingLayout'. */
  bool m_ApplyingLayout = false;

  /**
  * \brief Splitter proportions as they were before maximizing, restored on the
  *        way out and serialized in place of the live ones while maximized.
  *        Empty exactly when no cell is maximized.
  */
  std::vector<std::pair<QSplitter*, QList<int>>> m_PreMaximizeSizes;

  /** \brief Preference-backed default for the per-cell W/L corner readout. */
  bool m_LevelWindowReadoutVisible = true;

  /** \brief Editor-wide navigator mode (compact vs. expanded); the preference
   *         sets the default, the top-chrome toggle flips it live. Applied to
   *         cells created later, too. */
  bool m_NavigatorExpanded = false;

  /**
  * \brief Stashed layout-document `name` so it survives a load -> save
  *        round-trip. Empty when the source document had no `name` field;
  *        cleared by 'TearDownAllCells'.
  */
  std::string m_LayoutName;

  /** \brief Crosshair state every cell is created with, so a cell added after
   *         the editor configured its crosshairs matches the others. */
  bool m_CrosshairVisibility;
  bool m_CrosshairEnabled = false;
  unsigned int m_CrosshairGap = 32;

  /** \brief Whether the peek is up, which axis it emphasises, and the axis a
   *         started dwell will raise with. */
  bool m_SyncPeekVisible = false;
  std::optional<QmitkMxNSyncAxis> m_SyncPeekAxis;
  std::optional<QmitkMxNSyncAxis> m_SyncPeekPendingAxis;

  /** \brief The cell whose barcode the pointer was last on. */
  QString m_SyncPeekHoverCell;

  /** \brief The pointer rest that raises the peek, and the window a pointer off
   *         the strip must survive before it lowers again. */
  QTimer* m_SyncPeekDwell = nullptr;
  QTimer* m_SyncPeekGrace = nullptr;
  int m_SyncPeekDwellMs = 250;
  int m_SyncPeekGraceMs = 150;

  /** \brief Lower the hover peek and forget any pending dwell. */
  void LowerSyncPeek();

  /** \brief Push the plate state - up while arrange mode or the hover peek is,
   *         with the emphasis of whichever drives it - into every cell. */
  void RefreshSyncPeekPlates();

  QmitkMxNArrangeMode* m_ArrangeMode = nullptr;
  bool m_SyncPeekRefreshPending = false;

  /**
  * \brief The border colour each cell's stylesheet was last set to.
  *
  *   'QWidget::setStyleSheet' has no early-out on an unchanged string and
  *   repolishes the whole cell subtree, so 'RefreshFrameColors' consults this
  *   and writes only on a real change. Entries are dropped as cells are
  *   created and torn down, so a rebuilt cell that reuses an id always gets
  *   its first write.
  */
  std::map<QString, QColor> m_CellBorderColors;

  /** \brief The "loading layout" dialog and its two contents, null whenever
   *         none is up, and the cell count its bar is scaled against.
   *
   *   Deliberately a plain dialog rather than a QProgressDialog: that one owns
   *   the decision of when to make itself visible, through an internal
   *   estimation timer that 'setValue' restarts, and the whole difficulty here
   *   has been getting something on screen before the UI thread stops
   *   answering. This one shows when it is told to.
   */
  QPointer<QDialog> m_LoadDialog;
  QPointer<QLabel> m_LoadLabel;
  QPointer<QProgressBar> m_LoadBar;
  int m_LoadCellTarget = 0;
  int m_LoadCellsDone = 0;

  /** \brief Move the load dialog's bar to 'percent' under 'label'. */
  void StepLayoutLoadFeedback(int percent, const QString& label);

  /** \brief Account for one more cell built, and caption it. */
  void TickLayoutLoadFeedbackCell();

};

#endif
