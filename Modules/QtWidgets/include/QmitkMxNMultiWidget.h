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

public Q_SLOTS:

  // mouse events
  void wheelEvent(QWheelEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void moveEvent(QMoveEvent* e) override;
  void LoadLayout(const nlohmann::json* jsonData);
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

  QmitkAbstractMultiWidget::RenderWindowWidgetPointer CreateRenderWindowWidget();
  QmitkAbstractMultiWidget::RenderWindowWidgetPointer GetWindowFromIndex(size_t index);

  static nlohmann::json BuildJSONFromLayout(const QSplitter* splitter);
  QSplitter* BuildLayoutFromJSON(const nlohmann::json* jsonData, unsigned int* windowCounter, QSplitter* parentSplitter = nullptr);

  std::map < GroupSyncIndexType, std::unique_ptr<QmitkSynchronizedWidgetConnector> > m_SynchronizedWidgetConnectors;

  bool m_CrosshairVisibility;

};

#endif
