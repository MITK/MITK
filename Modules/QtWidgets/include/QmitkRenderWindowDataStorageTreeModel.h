/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowDataStorageTreeModel_h
#define QmitkRenderWindowDataStorageTreeModel_h

// render window manager UI model
#include "MitkQtWidgetsExports.h"

// render window manager module
#include "mitkRenderWindowLayerController.h"
#include "mitkRenderWindowLayerUtilities.h"

//mitk core
#include <mitkBaseRenderer.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>
#include <QmitkDataStorageTreeModelInternalItem.h>

/*
* @brief The 'QmitkRenderWindowDataStorageTreeModel' is a tree model derived from the 'QmitkAbstractDataStorageModel'.
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowDataStorageTreeModel : public QmitkAbstractDataStorageModel
{
  Q_OBJECT

public:

  QmitkRenderWindowDataStorageTreeModel(QObject* parent = nullptr);

  // override from 'QmitkAbstractDataStorageModel'
  /**
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void DataStorageChanged() override;
  /**
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodePredicateChanged() override;
  /**
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodeAdded(const mitk::DataNode* node) override;
  /**
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodeChanged(const mitk::DataNode* node) override;
  /**
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodeRemoved(const mitk::DataNode* node) override;

  // override from 'QAbstractItemModel'
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& parent) const override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  Qt::DropActions supportedDropActions() const override;
  Qt::DropActions supportedDragActions() const override;
  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList& indexes) const override;

  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

  void SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer);

  void SetCurrentRenderer(mitk::BaseRenderer* baseRenderer);
  mitk::BaseRenderer::Pointer GetCurrentRenderer() const;

private:

  void ResetTree();
  void UpdateModelData();
  /**
  * @brief Adjust the layer property according to the current tree.
  *        The function will set the "layer" property of each underlying data node so that it fits the
  *        the actual hierarchy represented by the current tree.
  */
  void AdjustLayerProperty();
  /**
  * @brief Fill a vector of tree items in a depth-first order (child-first).
  */
  void TreeToVector(QmitkDataStorageTreeModelInternalItem* parent, std::vector<QmitkDataStorageTreeModelInternalItem*>& treeAsVector) const;
  /**
  * @brief Add the given data node to the tree of the given renderer.
  *        The given renderer specifies the "layer"-property that is used for adding the new tree item
  *        to the tree. The "layer"-property may be different for each renderer resulting in a
  *        different tree for each renderer.
  *
  * @param dataNode   The data node that should be added.
  * @param renderer   The base renderer to which the data node should be added.
  */
  void AddNodeInternal(const mitk::DataNode* dataNode, const mitk::BaseRenderer* renderer);
  /**
  * @brief Remove the tree item that contains the given data node. Removing an item may
  *        leave the child items of the removed item without a parent. In this case
  *        the children have to be moved inside the tree so the tree has to be rebuild
  *        according to the current status of the data storage.
  *
  * @param dataNode   The data node that should be removed.
  */
  void RemoveNodeInternal(const mitk::DataNode* dataNode);

  mitk::DataNode* GetParentNode(const mitk::DataNode* node) const;
  QmitkDataStorageTreeModelInternalItem* GetItemByIndex(const QModelIndex& index) const;
  QModelIndex GetIndexByItem(QmitkDataStorageTreeModelInternalItem* item) const;

  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  QmitkDataStorageTreeModelInternalItem* m_Root;
  mitk::WeakPointer<mitk::BaseRenderer> m_BaseRenderer;

};

#endif
