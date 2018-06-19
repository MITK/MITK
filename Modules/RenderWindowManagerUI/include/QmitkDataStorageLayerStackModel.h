/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKDATASTORAGELAYERSTACKMODEL_H
#define QMITKDATASTORAGELAYERSTACKMODEL_H

// render window manager UI module
#include "MitkRenderWindowManagerUIExports.h"

// render window manager module
#include "mitkRenderWindowLayerUtilities.h"

// qt widgets module
#include <QmitkAbstractDataStorageModel.h>

/**
* @brief The 'QmitkDataStorageLayerStackModel' is a customized table model, derived from the 'QmitkAbstractDataStorageModel'.
*        It provides functions to accept a data storage and a node predicate in order to customize the model data nodes.
*        Furthermore it overrides the functions of 'QAbstractItemModel' to create a customized qt table model.
*        This model can be used in conjunction with a 'QmitkDataStorageSelectionConnector'.
*/
class MITKRENDERWINDOWMANAGERUI_EXPORT QmitkDataStorageLayerStackModel : public QmitkAbstractDataStorageModel
{
  Q_OBJECT

public:

  QmitkDataStorageLayerStackModel(QObject* parent = nullptr);

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

  void SetCurrentRenderer(const std::string& rendererName);

  mitk::BaseRenderer* GetCurrentRenderer() const;

  //////////////////////////////////////////////////////////////////////////
  /// overridden functions from QAbstractItemModel
  //////////////////////////////////////////////////////////////////////////
  QModelIndex index(int row, int column, const QModelIndex& parent) const;
  QModelIndex parent(const QModelIndex& child) const;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  //////////////////////////////////////////////////////////////////////////
  /// end override
  /////////////////////////////////////////////////////////////////////////

private:

  void UpdateModelData();

  mitk::WeakPointer<mitk::BaseRenderer> m_BaseRenderer;
  RenderWindowLayerUtilities::LayerStack m_TempLayerStack;

};

#endif // QMITKDATASTORAGELAYERSTACKMODEL_H
