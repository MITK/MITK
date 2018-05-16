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

#ifndef QMITKRENDERWINDOWDATAMODEL_H
#define QMITKRENDERWINDOWDATAMODEL_H

// render window manager module
#include "MitkRenderWindowManagerExports.h"
#include "mitkRenderWindowLayerUtilities.h"

//mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>

// qt
#include <QAbstractTableModel>

/*
* @brief This class extends the 'QAbstractTableModel' to meet the specific requirements of the QmitkRenderWindowDataModel.
*/
class MITKRENDERWINDOWMANAGER_EXPORT QmitkRenderWindowDataModel : public QAbstractTableModel
{
  Q_OBJECT

public:

  QmitkRenderWindowDataModel(QObject* parent = nullptr);
  ~QmitkRenderWindowDataModel() override;

  //////////////////////////////////////////////////////////////////////////
  /// overridden functions from QAbstractItemModel
  //////////////////////////////////////////////////////////////////////////
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  //////////////////////////////////////////////////////////////////////////
  /// end override
  /////////////////////////////////////////////////////////////////////////

  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);
  void SetCurrentRenderer(std::string rendererName);
  mitk::BaseRenderer* GetCurrentRenderer() const { return m_BaseRenderer.GetPointer(); }
  void DataChanged(const mitk::DataNode* dataNode);

private:

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::BaseRenderer::Pointer m_BaseRenderer;
  RenderWindowLayerUtilities::LayerStack m_TempLayerStack;

};

#endif // QMITKRENDERWINDOWDATAMODEL_H
