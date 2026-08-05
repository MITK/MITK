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
#include <map>

class QmitkMxNCellMapWidget;
class QmitkMultiWidgetLayoutSelectionWidget;
class QDialog;
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
 * The default face is visual: an interactive cell map mirroring the live
 * layout (tiles colored by navigation group, with a per-dimension sync
 * barcode), plus one card per group (hue, editable display name, dimension
 * checkboxes, re-converge, geometry reinit). Cells join a group by selecting
 * tiles and clicking the card's assign button, or by drag and drop in either
 * direction. Joining a group that synchronizes nothing yet links the
 * navigation bundle (pan/zoom/slice/crosshair) as the common-case default.
 *
 * The power-user face - the full per-dimension matrix with offset editors,
 * the complete v3-layout link model - is deliberately tucked behind the
 * "Advanced" toggle.
 *
 * The widget drives the multi widget's public sync API directly (a peer in
 * the same module; no delegate interface until a second consumer exists) and
 * follows the engine's change signals, so it doubles as a live legend while
 * docked. The embedded layout selection controls are exposed via
 * GetLayoutSelectionWidget so the hosting view can wire their apply/save/
 * load signals to the editor part exactly like the former toolbar popup.
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
   * \brief The embedded layout-shape controls (grid size, presets,
   *        save/load). The hosting view connects their signals to the
   *        editor part; they are not wired module-internally.
   */
  QmitkMultiWidgetLayoutSelectionWidget* GetLayoutSelectionWidget() const;

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
   * \brief Handle an axis-glyph click on a group's header barcode. Three cases:
   *        an empty group with no windows selected in the map toggles a per-group
   *        intent cache (applied to the first windows assigned, then cleared) and
   *        does not touch the engine; an empty group with a map selection
   *        bootstraps - it links the selected windows on the axis; a non-empty
   *        group homogenizes the axis over its members (link all / unlink all).
   *        'axisIndex' indexes the eight barcode axes (the seven
   *        QmitkMxNAllSyncDimensions, then data selection). Public so tests can
   *        drive the axis interaction directly.
   */
  void ToggleGroupAxis(const std::string& groupId, int axisIndex);

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

public Q_SLOTS:

  /** \brief Coalesced full refresh from the engine state. */
  void ScheduleRebuild();

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

  /** \brief Select the tile of the multi widget's active render window, so the
   *         map mirrors the editor's focus. */
  void SelectActiveWindowTile();

  void Rebuild();

  /** \brief Open the grid-shape picker (grid size, presets, save/load) in an
   *         on-demand modal dialog, re-parenting the shared picker into it and
   *         resetting its transient state so it opens fresh. */
  void ShowGridDialog();

  /** \brief Rebuild the advanced matrix from the current engine state. */
  void RebuildMatrixNow();

  /** \brief Rebuild the advanced matrix only while it is revealed (the "Advanced"
   *         toggle is on). Called from the structural rebuild paths and on reveal,
   *         never from the routine card refresh, so an editable combo the user is
   *         interacting with is not recreated mid-edit. */
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
  QmitkMxNCellMapWidget* m_CellMap;
  QVBoxLayout* m_GroupsLayout;    // card list inside the scrollable lower pane
  QTableWidget* m_Matrix;         // the advanced link matrix, inside m_MatrixPane
  QWidget* m_MatrixPane = nullptr;      // third splitter pane, hidden until "Advanced" is on
  QDialog* m_GridDialog = nullptr;      // on-demand modal grid-shape picker host
  QToolButton* m_AddGroupButton;
  QToolButton* m_AdvancedButton;
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
