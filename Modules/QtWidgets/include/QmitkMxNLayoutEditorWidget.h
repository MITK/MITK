/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNLayoutEditorWidget_h
#define QmitkMxNLayoutEditorWidget_h

#include <MitkQtWidgetsExports.h>

#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNGroupJoinMode.h>
#include <QmitkMxNSyncDimension.h>

#include <QPointer>
#include <QStringList>
#include <QWidget>

#include <array>
#include <functional>
#include <iosfwd>
#include <map>

class QmitkMultiWidgetLayoutSelectionWidget;
class QComboBox;
class QDialog;
class QDoubleSpinBox;
class QLabel;
class QSpinBox;
class QTabWidget;
class QTableWidget;
class QToolButton;
class QVBoxLayout;

/**
 * \brief The MxN editor's authoritative layout and synchronization editor.
 *
 * One structural surface for both jobs the layout document describes: the
 * cell arrangement (grid size, presets, save/load - the embedded layout
 * selection controls) and the per-dimension synchronization groups.
 *
 * The windows themselves are arranged in the display: while the hosting view
 * is visible, the editor is in arrange mode (QmitkMxNArrangeMode) and every
 * window's peek plate is where cells are selected and assigned. This widget
 * follows that selection. Two mutually exclusive configuration faces share a
 * tab widget, so only one is up at a time.
 *
 * "Sync groups" is the default face: one card per group (hue, editable
 * display name, dimension checkboxes, re-converge, geometry reinit). Cells
 * join a group by selecting them on their plates and using the card's menu, or
 * by drag and drop in either direction. Joining a group that synchronizes
 * nothing yet links the navigation bundle (pan/zoom/slice/crosshair) as the
 * common-case default.
 *
 * "Advanced" is the power-user face: the full per-dimension matrix with
 * offset editors, the complete v3-layout link model.
 *
 * The widget drives the multi widget's public sync API directly (a peer in
 * the same module; no delegate interface until a second consumer exists) and
 * follows the engine's change signals, so it doubles as a live legend while
 * docked. It also owns the layout document's own actions - the preset, load
 * and save controls in its header row, and the grid-shape picker behind "Edit
 * grid..." - and reports them as signals for the hosting view to apply to its
 * editor part, which is where the destructive paths are gated.
 *
 * UI-handler mutations are exposed as public methods so tests can drive the
 * widget against a real multi widget headlessly.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNLayoutEditorWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkMxNLayoutEditorWidget(QWidget* parent = nullptr);
  ~QmitkMxNLayoutEditorWidget() override;

  /**
   * \brief Attach to (or detach from, with nullptr) the MxN editor whose
   *        layout and sync state this widget edits.
   */
  void SetMultiWidget(QmitkMxNMultiWidget* multiWidget);
  QmitkMxNMultiWidget* GetMultiWidget() const;

  /**
   * \brief Takes a modifier-held press on one of the matrix's header viewports.
   *        Qt selects a whole line on a header press but resolves the selection
   *        command without the mouse event, so Ctrl would replace the selection
   *        rather than add to it. Plain presses are left to Qt, which keeps
   *        section drag-resize working.
   */
  bool eventFilter(QObject* watched, QEvent* event) override;

  /**
   * \brief The data storage the data-based layout option derives an
   *        arrangement from. Without it that option has nothing to offer.
   */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
   * \brief Add every given cell to 'group' on the group's currently
   *        synchronized dimensions (or the navigation bundle when the group
   *        synchronizes nothing yet), per the join 'mode'. Replace (the default)
   *        clears each cell's other-group ties first so it wholly joins the
   *        target; FillEmpty sets only its currently-unlinked axes;
   *        MergeOverwriteCollisions overwrites the group's axes but keeps the
   *        cell's links on axes the group does not cover.
   */
  void AssignCellsToGroup(const QStringList& windowIds, const std::string& group,
                          QmitkMxNGroupJoinMode mode = QmitkMxNGroupJoinMode::Replace);

  /**
   * \brief Link or unlink every current member cell of 'group' on one
   *        dimension. Membership is defined by the engine's links: a no-op
   *        for a group without members.
   */
  void ApplyDimensionToGroup(const std::string& group, QmitkMxNSyncDimension dimension, bool enabled);

  /**
   * \brief Link or unlink the data-selection axis for every current member
   *        cell of 'group'. Selection is single-valued per cell, so enabling
   *        moves each member's selection group to this group (last-writer-wins
   *        if a member already had a different one); disabling returns each
   *        member to the default selection group.
   */
  void ApplySelectionToGroup(const std::string& group, bool enabled);

  /**
   * \brief Handle an axis-glyph click on a group's header barcode. An empty
   *        group toggles a per-group intent cache (applied to the first windows
   *        assigned, then cleared) and does not touch the engine, whatever is
   *        selected; a non-empty group homogenizes the axis over its members
   *        (link all / unlink all). Public so tests can drive the axis
   *        interaction directly.
   */
  void ToggleGroupAxis(const std::string& groupId, QmitkMxNSyncAxis axis);

  /**
   * \brief Add a cell to / remove a cell from a group (see
   *        AssignCellsToGroup for the join semantics; leaving clears the
   *        cell's links to the group on every dimension).
   */
  void SetCellMembership(const QString& windowId, const std::string& group, bool member);

  /** \brief Link pan+zoom+slice+crosshair for every member cell of 'group'. */
  void LinkNavigationBundle(const std::string& group);

  /** \brief Re-converge every offset dimension (slice/zoom/pan) the group links. */
  void ReconvergeGroup(const std::string& group);

  /**
   * \brief Reinit the geometry-authority component of the group's first
   *        member cell (no-op for a group without members).
   */
  void ReinitGroupGeometry(const std::string& group);

  /** \brief Create a fresh synchronization group and return its id. */
  std::string CreateGroup();

  /**
   * \brief Link one cell on one axis to 'group'. An existing offset on the axis
   *        is carried over, so re-grouping a cell keeps the relationship the user
   *        authored. Public so the advanced matrix's edits are testable
   *        headlessly.
   */
  void SetCellAxisGroup(const QString& windowId, QmitkMxNSyncAxis axis, const std::string& group);

  /**
   * \brief Unlink one cell on one axis. Data selection has no unlinked state,
   *        so the selection axis returns the cell to the default group instead.
   */
  void ClearCellAxis(const QString& windowId, QmitkMxNSyncAxis axis);

  /**
   * \brief Set one cell's offset relative to its group's seed on an
   *        offset-bearing dimension, keeping its group. A no-op for a cell that
   *        is not linked on the dimension: an offset needs a seed to be relative
   *        to.
   */
  void SetCellDimensionOffset(const QString& windowId, QmitkMxNSyncDimension dimension,
                              const QmitkMxNMultiWidget::SyncOffset& offset);

  /**
   * \brief Spread slice offsets over the given cells in the order they are
   *        passed: the first gets 'from', each following one 'step' more. This is
   *        how a movie-frame layout showing slices -1 / 0 / +1 is authored.
   *        Restricted to slice because zoom composes multiplicatively and a pan
   *        ramp has no unambiguous direction in two dimensions.
   */
  void ApplySliceOffsetRamp(const QStringList& windowIds, int from, int step);

  /** \brief What the advanced matrix shows for one cell: the linked group's id
   *         (empty when the cell is unlinked on the axis) and the offset exactly as
   *         the chip renders it (empty when the offset is the neutral one).
   *         Read-only; public so the matrix's rendering is testable headlessly. */
  struct MatrixCellContent
  {
    std::string group;
    QString offset;
    bool highlighted = false;
  };
  MatrixCellContent AdvancedMatrixCell(const QString& windowId, QmitkMxNSyncAxis axis) const;

  /**
   * \brief Remove a synchronization group entirely (its cells are unsynchronized
   *        and revert to the default group). A no-op for the default group, which
   *        every cell falls back to and which is never removable. Public so tests
   *        and the card menu can drive it.
   */
  void DeleteGroup(const std::string& group);

  /**
   * \brief The group-perspective barcode slots for a group's header: one per
   *        axis (the seven dimensions then data selection). Each axis is
   *        tri-state over the group's member cells - a solid hue when all
   *        members link this group on that axis, a gap when none do, and the
   *        partial (dashed) state when only some do (reachable via per-cell
   *        edits or the advanced matrix). Distinct from the multiwidget's
   *        per-cell, binary BuildBarcodeSlots.
   */
  QList<QmitkMxNSyncBarcodeWidget::AxisSlot> BuildGroupBarcodeSlots(const std::string& group) const;

  /**
   * \brief Whether the current sync configuration is more than the trivial
   *        default. False only when the sole group is "main" and every cell
   *        resolves to Mono("main") - the fresh-cell state where windowing, LUT,
   *        and selection sit on "main" and nothing else is linked. True once a
   *        second group exists or any cell links other synchronization. The
   *        hosting view uses this to warn before a layout replace discards a
   *        configuration the user built; public so it is testable headlessly.
   */
  bool HasNonTrivialSyncConfig() const;

  /**
   * \brief Sync-highlight-on-hover. Resolve the cells sharing (group, axis),
   *        mark them in the matrix and hand them to the arrange mode, which
   *        bumps their frames in the display; ClearSyncHighlight removes the highlight.
   *        HighlightGroupAxis is driven by a group card's glyph hover;
   *        HighlightCellAxis by a window's glyph hover (it resolves the cell's
   *        group for that axis first - the per-dimension link, or the selection
   *        connector for the selection axis - then delegates, clearing when the
   *        cell syncs nothing there). Public so the resolution is testable
   *        headlessly.
   */
  void HighlightGroupAxis(const QString& group, QmitkMxNSyncAxis axis);
  void HighlightCellAxis(const QString& windowId, QmitkMxNSyncAxis axis);
  void ClearSyncHighlight();

public Q_SLOTS:

  /** \brief Coalesced full refresh from the engine state. */
  void ScheduleRebuild();

Q_SIGNALS:

  /**
   * \brief The layout actions the user asked for, for the hosting view to apply
   *        to its editor part. All but SaveLayout replace the whole arrangement
   *        and discard the current synchronization groups, so a host is expected
   *        to confirm them; the widget itself never applies them.
   */
  void LayoutSet(int row, int column);
  void SetDataBasedLayout(const QList<mitk::DataNode::Pointer>& nodes);
  void LoadLayout(const nlohmann::json* jsonData);

  /** \brief Write the current layout into the given stream. Connect directly:
   *         the stream is only valid for the duration of the emit. */
  void SaveLayout(std::ostream* outStream);

private:

  /**
   * \brief Refresh the group cards in place, or rebuild them, depending on what
   *        changed. A card never depends on the grid arrangement, so a link or
   *        membership change (the common case, e.g. a glyph toggle) only updates
   *        each existing card's contents - no widget teardown, no flicker. A full
   *        rebuild happens only when the set of groups changes: a group added or
   *        removed, or a whole layout replaced (load / REST push).
   */
  void RefreshOrRebuild();

  /** \brief Update every existing card's contents from the current engine state
   *         (glyph strip, member count, name, hue) without recreating widgets. */
  void RefreshCards();

  /** \brief Enable the add/remove row and column buttons only for a rectangular
   *         grid layout (the shape the trailing-edge grid ops can grow or shrink
   *         in place), gating on the tree-derived ResolveGridShape rather than
   *         the stored counts, and explain in the tooltip why they are off
   *         otherwise. */
  void UpdateGridButtons();

  /** \brief Select the multi widget's active render window, so the selection
   *         follows the editor's focus. */
  void SelectActiveWindow();

  /** \brief Say where windows are arranged, or why they cannot be while one
   *         is maximized. */
  void UpdateArrangeHint();

  /** \brief The arrange mode's window selection; empty without a multi widget. */
  QStringList SelectedWindowIds() const;

  void Rebuild();

  /** \brief Open the grid-shape picker (grid size, presets, save/load) in an
   *         on-demand modal dialog, re-parenting the shared picker into it and
   *         resetting its transient state so it opens fresh. */
  void ShowGridDialog();

  /** \brief Rebuild the advanced matrix from the current engine state: headers,
   *         rows, and the column widths measured from the content. Restores the
   *         selection onto the cells that survive. */
  void RebuildMatrixNow();

  /** \brief Build the matrix's action bar: the selection description, the group
   *         picker, and the offset and ramp editors. */
  QWidget* BuildMatrixActionBar();

  /** \brief Repaint the matrix's chips from the engine without touching its
   *         structure or its column widths, so an edit does not shift the grid
   *         under the pointer and the selection survives. */
  void RefreshMatrixCells();

  /** \brief The selected matrix cells as (window id, axis) pairs, in row then
   *         column order - row order being the layout's pre-order, which is the
   *         order a slice ramp spreads over. */
  std::vector<std::pair<QString, QmitkMxNSyncAxis>> MatrixSelection() const;

  /** \brief Re-describe the selection in the action bar and offer exactly the
   *         controls that apply to it: the group picker always, the offset editors
   *         only for a selection wholly on one offset-bearing dimension whose cells
   *         are all linked, the ramp only for slice. */
  void UpdateMatrixActionBar();

  /** \brief Assign every selected matrix cell to 'group', or unlink them all when
   *         it is empty. */
  void ApplyGroupToMatrixSelection(const std::string& group);

  /** \brief Re-read the offsets the ramp would write, in the order it writes
   *         them, into its preview. */
  void UpdateRampPreview();

  /** \brief Mark the matrix cells of 'windowIds' in 'column' as sharing the
   *         hovered synchronization; a negative column clears the marking. Driven
   *         from HighlightGroupAxis, so the display's frames and the matrix always
   *         show the same set. */
  void SetMatrixHighlight(const QStringList& windowIds, int column);

  /**
   * \brief The write half of SetCellAxisGroup / ClearCellAxis /
   *        SetCellDimensionOffset. These do not notify: the sync furniture
   *        (per-cell barcodes, frame colors, cell overlays) must not repaint a
   *        half-applied batch, so the caller calls RefreshSyncControls once when
   *        its batch is done - the same contract QmitkMxNMultiWidget's own
   *        mutators keep.
   */
  void WriteCellAxisGroup(const QString& windowId, QmitkMxNSyncAxis axis, const std::string& group);
  void WriteCellAxisCleared(const QString& windowId, QmitkMxNSyncAxis axis);
  void WriteCellDimensionOffset(const QString& windowId, QmitkMxNSyncDimension dimension,
                                const QmitkMxNMultiWidget::SyncOffset& offset);

  /** \brief Select a whole matrix row ('wholeRow') or column under a modifier:
   *         Ctrl adds the line, or takes it away when it is already wholly
   *         selected; Shift takes the block from the current line to this one. */
  void SelectMatrixLine(int section, bool wholeRow, Qt::KeyboardModifiers modifiers);

  /** \brief The group picker as a popup at the pointer, for editing the selection
   *         without travelling to the action bar. */
  void ShowMatrixGroupMenu(const QPoint& globalPos);

  /** \brief Select the rows of 'windowIds' in the matrix, mirroring the window
   *         selection. Guarded against the echo of its own mirroring. */
  void MirrorSelectionToMatrix(const QStringList& windowIds);

  /** \brief Whether the "Advanced" face is the raised tab. Independent of widget
   *         visibility, which a docked-away view would also report as false. */
  bool AdvancedFaceIsCurrent() const;

  /** \brief Rebuild the advanced matrix only while its face is up. Called from the
   *         structural rebuild paths and when the tab is raised, never from the
   *         routine card refresh, so an editable combo the user is interacting with
   *         is not recreated mid-edit. */
  void RefreshAdvancedMatrixIfVisible();

  /**
   * \brief Reconcile the displayed cards against the current group set instead of
   *        tearing them all down: delete cards for groups that are gone, build and
   *        insert cards for new groups, and re-order the survivors to match - each
   *        surviving card keeps its widget (a move, not a recreate), so adding or
   *        removing one group does not flicker or discard the others' state. The
   *        secondary advanced matrix is still fully rebuilt. */
  void ReconcileGroupCards(const std::vector<std::string>& currentIds,
                           const std::vector<QmitkMxNMultiWidget::SyncGroupInfo>& infos);
  QWidget* BuildGroupCard(const QmitkMxNMultiWidget::SyncGroupInfo& info);
  /** \brief Lay out the matrix's headers and its empty items for the current
   *         window set. RefreshMatrixCells fills the chips. */
  void RebuildMatrix(const std::vector<QmitkMxNMultiWidget::SyncGroupInfo>& infos,
                     const std::vector<QmitkMxNMultiWidget::WindowDescriptor>& descriptors);

  /** \brief Whether the group has a cache entry with at least one axis toggled
   *         on, so assigning windows should apply exactly the cache rather than
   *         SetCellMembership's nav-bundle default. */
  bool HasCachedGroupIntent(const std::string& group) const;

  /** \brief All member cells of the group over every dimension, pre-order. */
  std::vector<QString> GroupMembers(const std::string& group) const;

  /** \brief Dimensions the group currently links for at least one cell. */
  std::vector<QmitkMxNSyncDimension> GroupDimensions(const std::string& group) const;

  /** \brief Whether at least one member cell's selection group is this group. */
  bool GroupSelectionEnabled(const std::string& group) const;

  /** \brief Apply a group's axes to one cell per the join mode: Replace clears
   *         the cell's other ties first (selection reverts to the default),
   *         FillEmpty sets only currently-unlinked axes (and adopts the group's
   *         selection only for a cell resting on the default), Merge overwrites
   *         the given axes and keeps the rest. Does not refresh - the batch
   *         caller (AssignCellsToGroup / SetCellMembership) refreshes once. */
  void ApplyGroupAxesToCell(const QString& windowId, const std::string& group,
                            const std::vector<QmitkMxNSyncDimension>& dimensions,
                            bool includeSelection, QmitkMxNGroupJoinMode mode);

  static QString CellLabel(const QmitkMxNMultiWidget::WindowDescriptor& descriptor);

  QPointer<QmitkMxNMultiWidget> m_MultiWidget;

  QmitkMultiWidgetLayoutSelectionWidget* m_LayoutSelection;
  QLabel* m_ArrangeHint = nullptr;
  QVBoxLayout* m_GroupsLayout;    // card list inside the scrollable lower pane
  QTableWidget* m_Matrix;         // the advanced link matrix, inside m_MatrixPane
  QTabWidget* m_FacesTab = nullptr;     // hosts the "Sync groups" and "Advanced" faces
  QWidget* m_MatrixPane = nullptr;      // the "Advanced" face's page

  // The matrix's action bar: the only place the matrix is edited, so the table
  // itself stays a read surface and nothing covers the grid being worked on.
  QLabel* m_MatrixAxisIconLabel = nullptr;
  QLabel* m_MatrixSelectionLabel = nullptr;
  QComboBox* m_MatrixGroupPicker = nullptr;
  QToolButton* m_MatrixClearButton = nullptr;
  QWidget* m_MatrixOffsetRow = nullptr;
  QLabel* m_MatrixOffsetLabel = nullptr;
  QSpinBox* m_SliceOffsetEdit = nullptr;
  QDoubleSpinBox* m_ZoomOffsetEdit = nullptr;
  QLabel* m_PanOffsetLabel = nullptr;
  QDoubleSpinBox* m_PanOffsetXEdit = nullptr;
  QDoubleSpinBox* m_PanOffsetYEdit = nullptr;
  QLabel* m_RampLabel = nullptr;
  QSpinBox* m_RampFromEdit = nullptr;
  QLabel* m_RampStepLabel = nullptr;
  QSpinBox* m_RampStepEdit = nullptr;
  QToolButton* m_RampApplyButton = nullptr;
  QLabel* m_RampPreviewLabel = nullptr;

  // The matrix cells currently marked as sharing the hovered synchronization, so
  // a hover move repaints only what changes rather than the whole grid.
  std::vector<std::pair<int, int>> m_MatrixHighlighted;

  // Guards the window <-> matrix selection mirroring against its own echo: each
  // side emits on change, so an unguarded round trip would widen a column
  // selection in the matrix back to whole rows.
  bool m_MirroringSelection = false;
  QDialog* m_GridDialog = nullptr;      // on-demand modal grid-shape picker host
  QToolButton* m_AddGroupButton;
  QToolButton* m_MatrixAddGroupButton = nullptr;
  QToolButton* m_EditGridButton;
  QToolButton* m_AddRowButton;
  QToolButton* m_RemoveRowButton;
  QToolButton* m_AddColumnButton;
  QToolButton* m_RemoveColumnButton;

  // In-place refresh bookkeeping: one refresher per displayed card (keyed by
  // group id) and the group-id order currently shown, so RefreshOrRebuild can
  // tell a content change from a group-set change.
  std::map<std::string, std::function<void()>> m_CardRefreshers;
  // The card widget per group id, so the incremental reconcile can move, delete,
  // or keep individual cards (a QPointer so a card destroyed elsewhere reads back
  // as null rather than dangling).
  std::map<std::string, QPointer<QWidget>> m_CardsById;
  std::vector<std::string> m_DisplayedGroupIds;

  // The cell and group sets the advanced matrix currently reflects. The matrix is
  // rebuilt when either changes (a grid resize changes the cells / rows; a group
  // added or removed changes the columns' group dropdowns) but not on a pure
  // link-or-selection edit, which would tear down an open combo mid-edit.
  std::vector<QString> m_MatrixCellIds;
  std::vector<std::string> m_MatrixGroupIds;

  // Per-group, empty-group-only intent buffer: which of the eight axes (the
  // seven QmitkMxNAllSyncDimensions, then data selection) the user toggled on
  // while the group had no members. Applied to the first windows assigned, then
  // erased. Editor-held (not card-bound) so the const BuildGroupBarcodeSlots can
  // render from it and it survives the card reconcile; cleared on a whole-layout
  // reload / SetMultiWidget swap and when the group leaves the set.
  using AxisIntent = std::array<bool, QmitkMxNAllSyncDimensions.size() + 1>;
  std::map<std::string, AxisIntent> m_EmptyGroupAxisCache;

  bool m_RebuildPending = false;

};

#endif
