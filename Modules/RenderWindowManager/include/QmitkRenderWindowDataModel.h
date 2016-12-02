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

#ifndef QmitkRenderWindowDataModel_h
#define QmitkRenderWindowDataModel_h

// render window manager
#include "MitkRenderWindowManagerExports.h"

//mitk
#include <mitkDataStorage.h>

// qt
#include <QAbstractTableModel>

/*
* @brief This class extends the 'QAbstractItemModel' to meet the specific requirements of the QmitkRenderWindowDataModel.
*/
class MITKRENDERWINDOWMANAGER_EXPORT QmitkRenderWindowDataModel : public QAbstractTableModel
{
  Q_OBJECT

public:

  typedef std::string RenderWindowName;
  typedef std::vector<std::string> DataNodeNamesVector;
  // TODO: use std::map<RenderWindowName, std::map<int
  typedef std::map<RenderWindowName, DataNodeNamesVector> RenderWindowDataNodesMap;

  QmitkRenderWindowDataModel(QObject* parent = nullptr);
  virtual ~QmitkRenderWindowDataModel() {};

  //////////////////////////////////////////////////////////////////////////
  /// overridden functions from QAbstractItemModel
  //////////////////////////////////////////////////////////////////////////
  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
  virtual QVariant data(const QModelIndex &index, int role) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  //////////////////////////////////////////////////////////////////////////
  /// end override
  /////////////////////////////////////////////////////////////////////////

  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);
  void SetCurrentRenderWindowName(std::string renderWindowName);

  void InsertLayerNode(std::string dataNodeName, std::string renderer);
  void MoveNodeUp(int rowIndex, std::string dataNodeName, std::string renderer);
  void MoveNodeDown(int rowIndex, std::string dataNodeName, std::string renderer);

signals:
  void VisibilityChanged(int index, bool isDataNodeVisible);

private:

  mitk::DataStorage::Pointer m_DataStorage;
  // map from render window name / id to vector of data node names of the specific render window
  RenderWindowDataNodesMap m_RenderWindowDataNodesMap;
  RenderWindowName m_CurrentRenderWindowName;

};

#endif // QmitkRenderWindowDataModel_h
