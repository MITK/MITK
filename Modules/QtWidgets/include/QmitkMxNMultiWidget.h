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
#include <QmitkSynchronizedNodeSelectionWidget.h>
#include <QmitkSynchronizedWidgetConnector.h>

#include <nlohmann/json.hpp>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class QSplitter;

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

  void SetWidgetPlaneMode(int userMode) override;

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
  * \brief Returns the smallest positive index not already used by an existing
  *        synchronization group.
  *
  *   Useful for callers that want to allocate a fresh group without colliding
  *   with the set of existing groups (e.g. the "+" button in the per-cell
  *   utility widget, or external automation).
  */
  GroupSyncIndexType NextFreeSyncGroupIndex() const;

  /**
  * \brief Construct a render-window widget with a caller-supplied bare id.
  *
  *   The qualified name registered with `RenderingManager` is
  *   `<m_MultiWidgetName>.<bareId>`. The bare id is what the v2 layout
  *   format stores in the per-window 'id' field; the editor adds the prefix
  *   when registering with the rendering manager so that on-disk documents are
  *   independent of the editor instance's name.
  *
  *   This is the canonical creation API; callers that need a deterministic
  *   id (e.g. the layout applier) should go through it. Internal positional
  *   creation (used by 'SetLayout(r, c)') uses a private nullary overload that
  *   delegates here with a collision-free 'widget<i>' id.
  *
  * \param bareId  The in-document bare id (e.g. "widget0", "alpha"). Must
  *                be non-empty and must not collide with an existing widget's
  *                qualified name in this editor.
  *
  * \return  Shared pointer to the newly constructed render-window widget.
  *
  * \pre  bareId is non-empty                            (otherwise mitk::Exception)
  * \pre  no existing render-window uses the qualified name (otherwise mitk::Exception)
  *
  * \throws mitk::Exception on precondition violation.
  */
  RenderWindowWidgetPointer CreateRenderWindowWidget(const QString& bareId);

  /**
  * \brief Serialize the current layout tree to a v2.0 JSON document
  *        (always strict mode).
  *
  *   Group naming convention: engine-internal sync-group index 1 maps to the
  *   bare label "main"; other indices map to "g_<i>" where <i> is a counter
  *   assigned by pre-order encounter order over the cell list. Same engine
  *   state in produces the same group names out (round-trip stable).
  *
  *   See 'mxn-layout-v2.schema.json' for the document shape this method emits.
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
  *   field -- URL-segment-safe, unique within the document, used as the
  *   engine-side bare render-window name and as the URL path segment for
  *   REST sub-resources. 'displayName' is the optional `name` field -- a
  *   free-form human-readable label, empty when absent.
  */
  struct WindowDescriptor
  {
    QString id;              // bare layout id (no '<editorPrefix>.' prefix)
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
  * \brief Apply a v2.0 JSON document.
  *
  *   Tears down all existing render windows and rebuilds from scratch (no
  *   positional reuse). On failure during construction, rolls back to a
  *   single default cell and rethrows.
  *
  *   See 'mxn-layout-v2.schema.json' for the accepted document shape.
  *
  *   Group seeding: after the new cell tree is built, each group's runtime
  *   synchronized state (per-renderer 'visible' / 'layer') is seeded from
  *   the cell that appears first in document order whose links.selection
  *   names that group; remaining members are normalised to the seed. See
  *   the canonical rule on the schema's `groups` description.
  *
  * \param doc  A parsed v2.0 layout document.
  *
  * \pre  Must be called on the UI thread.
  *
  * \throws mitk::Exception on: version != "2.0"; structural shape violation;
  *         duplicate window ids; unknown view_direction; missing group
  *         reference in strict mode; nlohmann parse / type errors (rewrapped
  *         from 'nlohmann::json::exception' subtypes).
  */
  void ApplyLayout(const nlohmann::json& doc);

public Q_SLOTS:

  // mouse events
  void wheelEvent(QWheelEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void moveEvent(QMoveEvent* e) override;

  /**
  * \brief Slot wrapper around 'ApplyLayout'. Loads a v2.0 layout document
  *        (replaces the current cell tree).
  *
  * \param jsonData  Pointer to a parsed layout document. Must not be null
  *                  and must not represent a JSON null value.
  *
  * \pre   jsonData != nullptr                            (otherwise mitk::Exception)
  * \pre   !jsonData->is_null()                           (otherwise mitk::Exception)
  *
  * \throws mitk::Exception (rethrown from 'ApplyLayout') on null pointer,
  *         JSON null value, version != "2.0", structural shape violation,
  *         duplicate window ids, unknown view_direction, missing group
  *         reference in strict mode, or wrapped 'nlohmann::json::exception'
  *         subtypes.
  */
  void LoadLayout(const nlohmann::json* jsonData);

  /**
  * \brief Slot wrapper around 'SerializeLayout'. Writes the current layout
  *        as a v2.0 JSON document (pretty-printed) to 'outStream'.
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
  * \brief Slot connected to 'QmitkRenderWindowUtilityWidget::CreateNewSyncGroupRequested'.
  *        Allocates the next free group index via 'NextFreeSyncGroupIndex',
  *        creates the group, and assigns the requesting widget to it.
  */
  void OnCreateNewSyncGroupRequested(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget);

Q_SIGNALS:

  void WheelMoved(QWheelEvent *);
  void Moved();
  void UpdateUtilityWidgetViewPlanes();
  void LayoutChanged();
  void SyncGroupAdded(const GroupSyncIndexType index);

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

private:

  void SetLayoutImpl() override;
  void SetInteractionSchemeImpl() override { }

  /**
  * \brief Build the qualified RenderingManager name for a bare in-document
  *        name, i.e. '<m_MultiWidgetName>.<bareName>'.
  *
  *   Centralised so prefix concatenation lives in one place rather than being
  *   sprinkled across call sites.
  */
  QString MakeQualifiedName(const QString& bareName) const;

  /**
  * \brief Positional convenience overload used by 'SetLayout(r, c)',
  *        'InitializeMultiWidget', and 'SetDataBasedLayout'.
  *
  *   Picks the smallest non-negative 'i' such that 'widget<i>' is not already
  *   used as a bare id in this editor, then delegates to the explicit-id
  *   overload. This replaces the old 'widget<count>' form, which silently
  *   collided when custom-id'd cells already used the same index.
  */
  QmitkAbstractMultiWidget::RenderWindowWidgetPointer CreateRenderWindowWidget();

  QmitkAbstractMultiWidget::RenderWindowWidgetPointer GetWindowFromIndex(size_t index);

  /**
  * \brief Recursive serializer for a 'split' subtree. Emits a v2 JSON node.
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
  *   Centralizes the per-cell field lookup: bare id from the qualified
  *   render-window name (via 'StripEditorPrefix'), display name from the
  *   cell's own state, view-direction from the slice-navigation controller's
  *   default direction, selection group label from the engine's group-name
  *   registry. Both 'ListWindowDescriptors' and 'SerializeSplitter' use this
  *   so the per-cell descriptor logic lives in one place.
  *
  * \pre  cell != nullptr
  * \pre  cell's qualified name carries the editor prefix
  * \pre  cell's sync-group index has an entry in 'm_GroupNameByIndex'
  *
  * \throws mitk::Exception on precondition violation.
  */
  WindowDescriptor MakeWindowDescriptor(const QmitkRenderWindowWidget* cell) const;

  /**
  * \brief Recursive constructor for a v2 'split' subtree. Returns a freshly
  *        allocated QSplitter with the cell tree below.
  *
  *   Window leaves are created via 'CreateRenderWindowWidget(bareId)',
  *   then re-parented to the new splitter and moved into their target sync
  *   group via 'SetSynchronizationGroup'.
  */
  QSplitter* BuildSplitterFromJsonV2(const nlohmann::json& splitNode,
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
  * \param seedingOrder  Pre-order (bareWindowId, groupName) pairs captured
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
  * \brief Recovery path when 'ApplyLayout' construction fails part-way.
  *        Drains whatever was partially built and re-runs the default
  *        single-cell initialisation so the editor stays in a usable state.
  */
  void RollBackToSingleDefaultCell();

  /**
  * \brief Strip the editor's '<multiWidgetName>.' prefix from a qualified
  *        widget name to obtain the v2 bare id.
  *
  *        Throws if the prefix is absent (would indicate engine-state
  *        corruption — every cell registered through the canonical creation
  *        path carries the prefix).
  */
  QString StripEditorPrefix(const QString& qualifiedName) const;

  std::map < GroupSyncIndexType, std::unique_ptr<QmitkSynchronizedWidgetConnector> > m_SynchronizedWidgetConnectors;

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

  bool m_CrosshairVisibility;

};

#endif
