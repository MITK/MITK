/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowUtilityWidget_h
#define QmitkRenderWindowUtilityWidget_h

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkSynchronizedNodeSelectionWidget.h>
#include <QmitkSliceNavigationWidget.h>
#include <QmitkStepperAdapter.h>
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowViewDirectionController.h>

// qt
#include <QWidget>
#include <QHBoxLayout>
#include <QComboBox>

namespace mitk
{
  class DataStorage;
}

class QmitkRenderWindow;
class QToolButton;

/**
* \brief Utility widget that extends a QmitkRenderWindowWidget with window-specific controls.
*
* It offers to select the viewing direction of the window, as well as a QmitkSliceNavigationWidget
* to scroll through the current view direction.
* In addition, it contains a QmitkSynchronizedNodeSelectionWidget that controls renderer-specific
* properties and shown nodes, as well as a synchronization-group selector to share this state with
* other render windows.
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowUtilityWidget : public QWidget
{
	Q_OBJECT

public:

  QmitkRenderWindowUtilityWidget(
    QWidget* parent = nullptr,
    QmitkRenderWindow* renderWindow = nullptr,
    mitk::DataStorage* dataStorage = nullptr
  );

  ~QmitkRenderWindowUtilityWidget() override;

  using GroupSyncIndexType = int;

  /**
  * \brief Select the combobox row for the given synchronization group index.
  *
  * \param index  The 1-based group index. Must be >= 1 and must already be
  *               registered with this widget (i.e. 'OnSyncGroupAdded' has run
  *               for this index, or it was added by a prior 'SetSyncGroup').
  *
  * \pre  index >= 1                                          (otherwise mitk::Exception)
  * \pre  the group is present in this widget's combobox      (otherwise mitk::Exception)
  *
  * \throws mitk::Exception on precondition violation.
  */
  void SetSyncGroup(const GroupSyncIndexType index);

  /**
  * \brief Returns the currently selected group index, or '-1' when the combobox
  *        holds no selection.
  *
  *   '-1' is returned only when no group has yet been registered with this
  *   widget (the combobox is empty -- happens during initial construction
  *   before the first 'OnSyncGroupAdded' or 'SetSyncGroup'). After at least
  *   one group has been registered, the return value is always a valid group
  *   index >= 1.
  */
  GroupSyncIndexType GetSyncGroup() const;

  void SetGeometry(const itk::EventObject& event);
  QmitkSynchronizedNodeSelectionWidget* GetNodeSelectionWidget() const;

  /**
  * \brief Programmatically set the cell's view direction: applies the plane
  *        to the renderer and mirrors it in the selector WITHOUT emitting
  *        'ViewDirectionChanged'.
  *
  *   This is the propagation path: the owning multi widget uses it to relay
  *   an orientation-group change to member cells, so a relayed change can
  *   never re-trigger propagation. Only 'AnatomicalPlane::Axial' /
  *   'Coronal' / 'Sagittal' are supported; other planes are ignored.
  */
  void SetViewDirectionSelection(mitk::AnatomicalPlane viewDirection);

public Q_SLOTS:
  void UpdateViewPlaneSelection();

  /**
  * \brief Register a group row in the selector, labeled with the group's
  *        display name ("one vocabulary": the same label every surface
  *        shows, never a synthesized "Group N").
  */
  void OnSyncGroupAdded(const GroupSyncIndexType index, const QString& label);

  /** \brief Update a registered group row's label after a cosmetic rename. */
  void OnSyncGroupLabelChanged(const GroupSyncIndexType index, const QString& label);

  /**
  * \brief Mirror the editor-wide clean-view state into this cell's toggle
  *        button without re-emitting 'CleanViewToggled'.
  */
  void SetCleanViewChecked(bool checked);

Q_SIGNALS:

  void SynchronizationToggled(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget);
  void SyncGroupChanged(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget, GroupSyncIndexType index);
  /**
  * \brief Emitted when the user requests a new synchronization group via the '+' button.
  *        The owning multi widget allocates a free index and assigns this cell to it.
  */
  void CreateNewSyncGroupRequested(QmitkSynchronizedNodeSelectionWidget* synchronizedWidget);
  void SetDataSelection(const QList<mitk::DataNode::Pointer>& newSelection);

  /**
  * \brief Emitted after the cell's view direction changed through this
  *        widget's selector. The programmatic 'SetViewDirectionSelection'
  *        path stays silent, so listeners (the multi widget's orientation
  *        propagation) only see user-driven changes.
  */
  void ViewDirectionChanged(mitk::AnatomicalPlane viewDirection);

  /**
  * \brief Emitted when the user toggles clean-view mode in this cell. The
  *        mode is editor-wide; the owning multi widget applies it to every
  *        cell and mirrors it back via 'SetCleanViewChecked'.
  */
  void CleanViewToggled(bool cleanView);

  /**
  * \brief Emitted when the user asks for the editor-wide layout editor from
  *        this cell's "Sync" button; the owning multi widget relays it to
  *        whoever hosts the view.
  */
  void LayoutEditorRequested();

private Q_SLOTS:

  void OnSyncGroupSelectionChanged(int index);
  void OnNodeSelectionWidgetSyncGroupChanged(int index);

private:

  mitk::BaseRenderer* m_BaseRenderer;
  QmitkSynchronizedNodeSelectionWidget* m_NodeSelectionWidget;
  QComboBox* m_SyncGroupSelector;
  QToolButton* m_NewSyncGroupButton;
  QToolButton* m_CleanViewButton;
  QmitkSliceNavigationWidget* m_SliceNavigationWidget;
  QmitkStepperAdapter* m_StepperAdapter;
  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;
  QComboBox* m_ViewDirectionSelector;

  void ChangeViewDirection(const QString& viewDirection);

};

#endif
