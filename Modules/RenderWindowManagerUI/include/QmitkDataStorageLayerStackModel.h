/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageLayerStackModel_h
#define QmitkDataStorageLayerStackModel_h

// render window manager UI module
#include <MitkRenderWindowManagerUIExports.h>

// render window manager module
#include <mitkRenderWindowLayerUtilities.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

/**
 * \brief Table model that displays the render window layer stack for a specific renderer.
 *
 * QmitkDataStorageLayerStackModel is a customized two-column table model derived from
 * QmitkAbstractDataStorageModel. It displays data nodes assigned to a specific render window
 * (base renderer) sorted by their layer property, with a visibility checkbox in the first
 * column and the node name in the second column.
 *
 * The model updates automatically when the data storage changes, nodes are modified or
 * removed, or the current renderer is switched. It can be used in conjunction with a
 * QmitkDataStorageSelectionConnector for selection synchronization.
 *
 * \sa QmitkAbstractDataStorageModel, QmitkRenderWindowDataStorageListModel,
 *     QmitkRenderWindowDataStorageInspector, mitk::RenderWindowLayerUtilities
 */
class MITKRENDERWINDOWMANAGERUI_EXPORT QmitkDataStorageLayerStackModel : public QmitkAbstractDataStorageModel
{
  Q_OBJECT

public:

  /**
   * \brief Construct the layer stack model.
   *
   * \param[in] parent Optional parent QObject.
   */
  QmitkDataStorageLayerStackModel(QObject* parent = nullptr);

  /**
   * \brief Called when the data storage has been changed. Refreshes the model data.
   */
  void DataStorageChanged() override;

  /**
   * \brief Called when the node predicate has been changed. Refreshes the model data.
   */
  void NodePredicateChanged() override;

  /**
   * \brief Called when a data node has been added to the data storage.
   *
   * \param[in] node The data node that was added.
   *
   * \note This is a no-op; layers are only added after an explicit user action.
   */
  void NodeAdded(const mitk::DataNode* node) override;

  /**
   * \brief Called when a data node has been changed. Refreshes the model data.
   *
   * \param[in] node The data node that was changed.
   */
  void NodeChanged(const mitk::DataNode* node) override;

  /**
   * \brief Called when a data node has been removed from the data storage. Refreshes the model data.
   *
   * \param[in] node The data node that was removed.
   */
  void NodeRemoved(const mitk::DataNode* node) override;

  /**
   * \brief Set the current renderer by its name and refresh the layer stack.
   *
   * Looks up the base renderer by name and rebuilds the model to show only the
   * layer stack for that renderer.
   *
   * \param[in] rendererName The name of the base renderer to display layers for.
   * \pre The data storage must be set and not expired.
   */
  void SetCurrentRenderer(const std::string& rendererName);

  /**
   * \brief Get the currently active base renderer.
   *
   * \return Pointer to the current mitk::BaseRenderer, or \c nullptr if none is set or it has expired.
   */
  mitk::BaseRenderer* GetCurrentRenderer() const;

  /**
   * \brief Create a model index for the given row and column.
   *
   * \param[in] row The row number.
   * \param[in] column The column number (0 = visibility, 1 = node name).
   * \param[in] parent The parent index (must be invalid for this flat model).
   * \return A valid QModelIndex if the row and column are within range, otherwise an invalid index.
   */
  QModelIndex index(int row, int column, const QModelIndex& parent) const;

  /**
   * \brief Return the parent index (always invalid, since this is a flat table model).
   *
   * \param[in] child The child index (unused).
   * \return An invalid QModelIndex.
   */
  QModelIndex parent(const QModelIndex& child) const;

  /**
   * \brief Return item flags for the given index.
   *
   * Column 0 (visibility) has Qt::ItemIsUserCheckable in addition to enabled/selectable flags.
   *
   * \param[in] index The model index to query.
   * \return The item flags for the given index, or Qt::NoItemFlags if the index is invalid.
   */
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * \brief Return header data for the table.
   *
   * Column 0 header is "Visibility", column 1 header is "Data node".
   *
   * \param[in] section The column number.
   * \param[in] orientation The header orientation (only Qt::Horizontal is handled).
   * \param[in] role The data role (only Qt::DisplayRole is handled).
   * \return The header text, or an invalid QVariant for unhandled parameters.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * \brief Return the number of rows (layers) in the model.
   *
   * \param[in] parent The parent index. Must be invalid for a valid count; returns 0 if valid.
   * \return The number of data nodes in the current layer stack.
   */
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Return the number of columns (always 2: visibility and node name).
   *
   * \param[in] parent The parent index. Must be invalid for a valid count; returns 0 if valid.
   * \return 2.
   */
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Return data for the given index and role.
   *
   * Supported roles:
   * - Qt::CheckStateRole (column 0): Node visibility in the current renderer.
   * - Qt::DisplayRole (column 1): Data node name.
   * - Qt::ToolTipRole: Descriptive tooltip for each column.
   * - QmitkDataNodeRole: mitk::DataNode::Pointer wrapped in a QVariant.
   * - QmitkDataNodeRawPointerRole: Raw mitk::DataNode pointer wrapped in a QVariant.
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

private:

  void UpdateModelData();

  mitk::WeakPointer<mitk::BaseRenderer> m_BaseRenderer;
  RenderWindowLayerUtilities::LayerStack m_TempLayerStack;

};

#endif
