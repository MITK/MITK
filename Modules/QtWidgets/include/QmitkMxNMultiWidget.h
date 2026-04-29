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
  * \param index             The 1-based group index. Must be >= 1.
  *
  * \pre  index >= 1                       (otherwise mitk::Exception)
  * \pre  GetDataStorage() != nullptr      (otherwise mitk::Exception)
  *
  * \throws mitk::Exception on precondition violation.
  */
  void AddSynchronizationGroup(const GroupSyncIndexType index);

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
  * \brief Construct a render-window widget with a caller-supplied bare name.
  *
  *   The qualified name registered with `RenderingManager` is
  *   `<m_MultiWidgetName>.<bareName>`. The bare name is what the v2 layout
  *   format stores in the per-window 'name' field; the editor adds the prefix
  *   when registering with the rendering manager so that on-disk documents are
  *   independent of the editor instance's name.
  *
  *   This is the canonical creation API; callers that need a deterministic
  *   name (e.g. the layout applier) should go through it. Internal positional
  *   creation (used by 'SetLayout(r, c)') uses a private nullary overload that
  *   delegates here with a collision-free 'widget<i>' name.
  *
  * \param bareName  The in-document bare name (e.g. "widget0", "alpha"). Must
  *                  be non-empty and must not collide with an existing widget's
  *                  qualified name in this editor.
  *
  * \return  Shared pointer to the newly constructed render-window widget.
  *
  * \pre  bareName is non-empty                          (otherwise mitk::Exception)
  * \pre  no existing render-window uses the qualified name (otherwise mitk::Exception)
  *
  * \throws mitk::Exception on precondition violation.
  */
  RenderWindowWidgetPointer CreateRenderWindowWidget(const QString& bareName);

  /**
  * \brief Serialize the current layout tree to a v2.0 JSON document
  *        (always strict mode).
  *
  *   Group naming convention: engine-internal sync-group index 1 maps to the
  *   bare name "main"; other indices map to "g_<i>" where <i> is a counter
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
  * \brief Apply a v2.0 JSON document.
  *
  *   Tears down all existing render windows and rebuilds from scratch (no
  *   positional reuse). On failure during construction, rolls back to a
  *   single default cell and rethrows.
  *
  *   See 'mxn-layout-v2.schema.json' for the accepted document shape.
  *
  * \param doc  A parsed v2.0 layout document.
  *
  * \pre  Must be called on the UI thread.
  *
  * \throws mitk::Exception on: version != "2.0"; structural shape violation;
  *         duplicate window names; unknown view_direction; missing group
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
  * \param jsonData  Pointer to a parsed layout document. Must not be null.
  *
  * \pre   jsonData != nullptr                            (otherwise mitk::Exception)
  *
  * \throws mitk::Exception (rethrown from 'ApplyLayout') on null input,
  *         version != "2.0", structural shape violation, duplicate window
  *         names, unknown view_direction, missing group reference in strict
  *         mode, or wrapped 'nlohmann::json::exception' subtypes.
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
  *   used as a bare name in this editor, then delegates to the explicit-name
  *   overload. This replaces the old 'widget<count>' form, which silently
  *   collided when custom-named cells already used the same index.
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
  * \brief Recursive constructor for a v2 'split' subtree. Returns a freshly
  *        allocated QSplitter with the cell tree below.
  *
  *   Window leaves are created via 'CreateRenderWindowWidget(bareName)',
  *   then re-parented to the new splitter and moved into their target sync
  *   group via 'SetSynchronizationGroup'.
  */
  QSplitter* BuildSplitterFromJsonV2(const nlohmann::json& splitNode,
                                     const std::map<std::string, GroupSyncIndexType>& nameToInt,
                                     QSplitter* parentSplitter);

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
  *        widget name to obtain the v2 bare name.
  *
  *        Throws if the prefix is absent (would indicate engine-state
  *        corruption — every cell registered through the canonical creation
  *        path carries the prefix).
  */
  QString StripEditorPrefix(const QString& qualifiedName) const;

  std::map < GroupSyncIndexType, std::unique_ptr<QmitkSynchronizedWidgetConnector> > m_SynchronizedWidgetConnectors;

  bool m_CrosshairVisibility;

};

#endif
