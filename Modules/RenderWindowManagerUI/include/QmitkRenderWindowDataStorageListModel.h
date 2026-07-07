/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowDataStorageListModel_h
#define QmitkRenderWindowDataStorageListModel_h

// render window manager UI model
#include <MitkRenderWindowManagerUIExports.h>

// render window manager module
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowLayerUtilities.h>

//mitk core
#include <mitkBaseRenderer.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

/**
 * \brief List model that displays the layer stack for a specific render window with drag-and-drop support.
 *
 * QmitkRenderWindowDataStorageListModel is a single-column list model derived from
 * QmitkAbstractDataStorageModel. It shows data nodes assigned to a render window sorted
 * by their layer property, with a visibility checkbox, node icon, and node name for each entry.
 *
 * The model supports drag-and-drop of data node pointers for reordering layers. When a node
 * is dropped onto another, the RenderWindowLayerController is used to move it to the target
 * layer position. Newly added nodes are automatically inserted into all controlled render
 * windows at the top layer position.
 *
 * \sa QmitkAbstractDataStorageModel, QmitkDataStorageLayerStackModel,
 *     QmitkRenderWindowDataStorageInspector, mitk::RenderWindowLayerController,
 *     mitk::RenderWindowLayerUtilities
 */
class MITKRENDERWINDOWMANAGERUI_EXPORT QmitkRenderWindowDataStorageListModel : public QmitkAbstractDataStorageModel
{
  Q_OBJECT

public:

  /**
   * \brief Construct the list model.
   *
   * Initializes the internal RenderWindowLayerController.
   *
   * \param[in] parent Optional parent QObject.
   */
  QmitkRenderWindowDataStorageListModel(QObject* parent = nullptr);

  /**
   * \brief Called when the data storage has been changed.
   *
   * Updates the layer controller's data storage reference and refreshes the model.
   */
  void DataStorageChanged() override;

  /**
   * \brief Called when the node predicate has been changed. Refreshes the model data.
   */
  void NodePredicateChanged() override;

  /**
   * \brief Called when a data node has been added to the data storage.
   *
   * Inserts the new node into all controlled render windows at the top layer position,
   * then refreshes the model.
   *
   * \param[in] node The data node that was added.
   */
  void NodeAdded(const mitk::DataNode* node) override;

  /**
   * \brief Called when a data node has been changed.
   *
   * \param[in] node The data node that was changed.
   *
   * \note Currently a no-op because the NodeChanged event is sent too frequently.
   */
  void NodeChanged(const mitk::DataNode* node) override;

  /**
   * \brief Called when a data node has been removed from the data storage. Refreshes the model.
   *
   * \param[in] node The data node that was removed.
   */
  void NodeRemoved(const mitk::DataNode* node) override;

  /**
   * \brief Create a model index for the given row and column.
   *
   * \param[in] row The row number.
   * \param[in] column The column number (always 0 for this single-column model).
   * \param[in] parent The parent index (must be invalid for this flat model).
   * \return A valid QModelIndex if the position is within range, otherwise an invalid index.
   */
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Return the parent index (always invalid, since this is a flat list model).
   *
   * \param[in] child The child index (unused).
   * \return An invalid QModelIndex.
   */
  QModelIndex parent(const QModelIndex& child) const override;

  /**
   * \brief Return the number of rows (layers) in the model.
   *
   * \param[in] parent The parent index. Must be invalid for a valid count; returns 0 if valid.
   * \return The number of data nodes in the current layer stack.
   */
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Return the number of columns (always 1).
   *
   * \param[in] parent The parent index. Must be invalid for a valid count; returns 0 if valid.
   * \return 1.
   */
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Return data for the given index and role.
   *
   * Supported roles:
   * - Qt::CheckStateRole: Node visibility in the current renderer.
   * - Qt::DisplayRole: Data node name.
   * - Qt::ToolTipRole: "Name of the data node."
   * - Qt::DecorationRole: Node type icon from QmitkNodeDescriptorManager.
   * - Qt::UserRole / QmitkDataNodeRawPointerRole: Raw mitk::DataNode pointer.
   * - QmitkDataNodeRole: mitk::DataNode::Pointer wrapped in a QVariant.
   *
   * \param[in] index The model index.
   * \param[in] role The data role.
   * \return The requested data, or an invalid QVariant.
   */
  QVariant data(const QModelIndex& index, int role) const override;

  /**
   * \brief Set data for the given index and role (visibility toggling).
   *
   * Only handles Qt::CheckStateRole to toggle the visibility of the data node in
   * the current renderer. Triggers a render window update on success.
   *
   * \param[in] index The model index of the item to modify.
   * \param[in] value The new value (Qt::CheckState as int).
   * \param[in] role The data role (only Qt::CheckStateRole is handled).
   * \return \c true if the visibility was successfully changed, \c false otherwise.
   */
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  /**
   * \brief Return item flags for the given index.
   *
   * Valid items are enabled, selectable, checkable, draggable, and droppable.
   * An invalid index is only droppable (to allow drops on empty space).
   *
   * \param[in] index The model index to query.
   * \return The item flags.
   */
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * \brief Return supported drop actions (Copy and Move).
   *
   * \return Qt::CopyAction | Qt::MoveAction.
   */
  Qt::DropActions supportedDropActions() const override;

  /**
   * \brief Return supported drag actions (Copy and Move).
   *
   * \return Qt::CopyAction | Qt::MoveAction.
   */
  Qt::DropActions supportedDragActions() const override;

  /**
   * \brief Return the list of supported MIME types for drag-and-drop.
   *
   * Includes QmitkMimeTypes::DataNodePtrs in addition to the default MIME types.
   *
   * \return List of supported MIME type strings.
   */
  QStringList mimeTypes() const override;

  /**
   * \brief Create MIME data for the given model indices.
   *
   * Serializes the raw data node pointers from the selected indices into a QByteArray
   * using the QmitkMimeTypes::DataNodePtrs format.
   *
   * \param[in] indexes The list of model indices to serialize.
   * \return Pointer to the created QMimeData. Ownership is transferred to the caller.
   */
  QMimeData* mimeData(const QModelIndexList& indexes) const override;

  /**
   * \brief Handle dropped MIME data to reorder layers.
   *
   * When data nodes are dropped onto a target item, uses the RenderWindowLayerController
   * to move the dropped nodes to the layer position of the target.
   *
   * \param[in] data The MIME data containing serialized data node pointers.
   * \param[in] action The drop action.
   * \param[in] row The target row (unused).
   * \param[in] column The target column.
   * \param[in] parent The model index of the drop target.
   * \return \c true if the drop was successfully handled, \c false otherwise.
   */
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

  /**
   * \brief Set the list of controlled renderers and initialize layer data for all nodes.
   *
   * Iterates over all data nodes in the data storage and inserts each one into all
   * controlled render windows via AddDataNodeToAllRenderer().
   *
   * \param[in] controlledRenderer Vector of base renderers to manage.
   */
  void SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer);

  /**
   * \brief Set the current base renderer and refresh the model.
   *
   * Switches the model to display the layer stack of the given renderer.
   *
   * \param[in] baseRenderer The base renderer whose layer stack should be displayed.
   */
  void SetCurrentRenderer(mitk::BaseRenderer* baseRenderer);

  /**
   * \brief Get the currently active base renderer.
   *
   * \return Smart pointer to the current mitk::BaseRenderer, or \c nullptr if none is set or it has expired.
   */
  mitk::BaseRenderer::Pointer GetCurrentRenderer() const;

  /**
   * \brief Insert the given data node into all controlled render windows at the top layer.
   *
   * Uses the RenderWindowLayerController to insert the node at the highest layer position
   * in each managed render window.
   *
   * \param[in] dataNode The data node to insert.
   */
  void AddDataNodeToAllRenderer(mitk::DataNode* dataNode);

private:

  void UpdateModelData();

  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  mitk::WeakPointer<mitk::BaseRenderer> m_BaseRenderer;
  mitk::RenderWindowLayerUtilities::LayerStack m_LayerStack;

};

#endif
