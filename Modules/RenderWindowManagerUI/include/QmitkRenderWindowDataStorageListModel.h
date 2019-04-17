/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKDATASTORAGERENDERWINDOWLISTMODEL_H
#define QMITKDATASTORAGERENDERWINDOWLISTMODEL_H

// render window manager UI model
#include "MitkRenderWindowManagerUIExports.h"

// render window manager module
#include "mitkRenderWindowLayerController.h"
#include "mitkRenderWindowLayerUtilities.h"

//mitk core
#include <mitkBaseRenderer.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

/*
* @brief The 'QmitkDataStorageRenderWindowListModel' is a list model derived from the 'QmitkAbstractDataStorageModel'.
*/
class MITKRENDERWINDOWMANAGERUI_EXPORT QmitkRenderWindowDataStorageListModel : public QmitkAbstractDataStorageModel
{
  Q_OBJECT

public:

  QmitkRenderWindowDataStorageListModel(QObject* parent = nullptr);

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
  QModelIndex parent(const QModelIndex& child) const override;

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
  mitk::BaseRenderer* GetCurrentRenderer() const;

  /**
  * @brief Use the RenderWindowLayerController to insert the given data node into all controlled render windows.
  *        The new node is placed on top of all existing layer nodes in the render window.
  *
  * @param dataNode   The data node that should be inserted.
  */
  void AddDataNodeToAllRenderer(mitk::DataNode* dataNode);

private:

  void UpdateModelData();

  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  mitk::WeakPointer<mitk::BaseRenderer> m_BaseRenderer;
  mitk::RenderWindowLayerUtilities::LayerStack m_LayerStack;

};

#endif // QMITKDATASTORAGERENDERWINDOWLISTMODEL_H
