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
#include "mitkRenderWindowLayerUtilities.h"

//mitk core
#include <mitkBaseRenderer.h>

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

/*
* @brief The 'QmitkDataStorageRenderWindowListModel' is a list model derived from the 'QmitkAbstractDataStorageModel'.
*/
class MITKRENDERWINDOWMANAGERUI_EXPORT QmitkDataStorageRenderWindowListModel : public QmitkAbstractDataStorageModel
{
  Q_OBJECT

public:

  QmitkDataStorageRenderWindowListModel(QObject* parent = nullptr);
  ~QmitkDataStorageRenderWindowListModel() override;

  // override from 'QmitkAbstractDataStorageModel'
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void DataStorageChanged() override;
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodePredicateChanged() override;
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodeAdded(const mitk::DataNode* node) override;
  /*
  * @brief See 'QmitkAbstractDataStorageModel'
  */
  void NodeChanged(const mitk::DataNode* node) override;
  /*
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

  void SetCurrentRenderer(mitk::BaseRenderer* baseRenderer);
  mitk::BaseRenderer* GetCurrentRenderer() const { return m_BaseRenderer.GetPointer(); }

private:

  void UpdateModelData();

  mitk::BaseRenderer::Pointer m_BaseRenderer;
  mitk::RenderWindowLayerUtilities::LayerStack m_TempLayerStack;

};

#endif // QMITKDATASTORAGERENDERWINDOWLISTMODEL_H
