/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSynchronizedNodeSelectionWidget_h
#define QmitkSynchronizedNodeSelectionWidget_h

#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkBaseRenderer.h>

// qt widgets module
#include <QmitkAbstractNodeSelectionWidget.h>
#include <QmitkRenderWindowDataNodeTableModel.h>
#include <memory>

namespace Ui
{
  class QmitkSynchronizedNodeSelectionWidget;
}

/**
* \brief Node selection widget with renderer-specific functionality and synchronization support.
*
* The QmitkSynchronizedNodeSelectionWidget implements QmitkAbstractNodeSelectionWidget
* by providing a table view, using a QmitkRenderWindowDataNodeTableModel and extending it
* with base renderer-specific functionality.
*
* Given a base renderer, the selection widget is able to display and access render window specific properties
* of the selected nodes. It can be connected with other QmitkSynchronizedNodeSelectionWidgets to synchronize
* their state.
* The widget can be used to decide if all data nodes of the data storage should be selected or
* only an individually selected set of nodes, defined by a QmitkNodeSelectionDialog.
* If individual nodes are selected / removed from the selection, the widget can inform other
* QmitkSynchronizedNodeSelectionWidget instances about the current selection, if desired.
* Additionally the widget allows to reinitialize the corresponding base renderer with a specific
* data node geometry.
*/
class MITKQTWIDGETS_EXPORT QmitkSynchronizedNodeSelectionWidget : public QmitkAbstractNodeSelectionWidget
{
  Q_OBJECT

public:

  QmitkSynchronizedNodeSelectionWidget(QWidget* parent);
  ~QmitkSynchronizedNodeSelectionWidget();

  using NodeList = QmitkAbstractNodeSelectionWidget::NodeList;

  /**
   * \brief Sets the base renderer that this widget controls.
   * \param[in] baseRenderer Pointer to the base renderer.
   */
  void SetBaseRenderer(mitk::BaseRenderer* baseRenderer);

  /** \brief Returns the internal data node table model. */
  QmitkRenderWindowDataNodeTableModel* GetStorageModel() const;

  /**
   * \brief Sets whether all nodes should be selected.
   * \param[in] selectAll If true, selects all nodes from the data storage.
   */
  void SetSelectAll(bool selectAll);
  /** \brief Returns whether the "select all" mode is active. */
  bool GetSelectAll() const;
  /** \brief Selects all nodes from the data storage. */
  void SelectAll();
  /** \brief Type alias for the synchronization group index. */
  using GroupSyncIndexType = int;
  /**
   * \brief Sets the synchronization group for this widget.
   * \param[in] index The synchronization group index.
   */
  void SetSyncGroup(const GroupSyncIndexType index);
  /** \brief Returns the current synchronization group index. */
  GroupSyncIndexType GetSyncGroup() const;

Q_SIGNALS:

  void SelectionModeChanged(bool selectAll);
  void DeregisterSynchronization();
  void NodeVisibilityChanged(mitk::DataNode::Pointer node, const bool visibility);
  /**
   * \brief Emitted when the synchronization group index actually changes.
   *
   * Listeners (typically the owning utility widget) use this to keep view-side
   * state -- e.g. a group-selector combobox -- in sync with the widget's
   * authoritative 'm_SyncGroupIndex'. Not emitted when 'SetSyncGroup' is called
   * with the value the widget already holds.
   */
  void SyncGroupIndexChanged(GroupSyncIndexType index);

public Q_SLOTS:
  void SetSelection(const NodeList& newSelection);
  void SetNodeVisibility(mitk::DataNode::Pointer node, const bool visibility);

private Q_SLOTS:

  void OnSelectionModeChanged(bool selectAll);
  void OnEditSelection();
  void OnTableClicked(const QModelIndex& index);

protected:

  void SetUpConnections();
  void Initialize();

  void UpdateInfo() override;
  void OnDataStorageChanged() override;
  void OnNodePredicateChanged() override;
  void ReviseSelectionChanged(const NodeList& oldInternalSelection, NodeList& newInternalSelection) override;
  void OnInternalSelectionChanged() override;
  void OnNodeAddedToStorage(const mitk::DataNode* node) override;
  void OnNodeModified(const itk::Object* caller, const itk::EventObject& event) override;

private:

  void ReinitNode(const mitk::DataNode* dataNode);
  void RemoveFromInternalSelection(mitk::DataNode* dataNode);
  bool IsParentNodeSelected(const mitk::DataNode* dataNode) const;
  void DeselectNode(mitk::DataNode* dataNode);

  std::unique_ptr<Ui::QmitkSynchronizedNodeSelectionWidget> m_Controls;
  mitk::WeakPointer<mitk::BaseRenderer> m_BaseRenderer;

  std::unique_ptr<QmitkRenderWindowDataNodeTableModel> m_StorageModel;
  GroupSyncIndexType m_SyncGroupIndex; // -1 = unassigned

};

#endif
