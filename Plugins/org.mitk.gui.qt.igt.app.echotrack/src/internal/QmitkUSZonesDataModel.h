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

#ifndef QMITKUSZONESDATAMODEL_H
#define QMITKUSZONESDATAMODEL_H

#include <mitkCommon.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkNumericTypes.h>

#include <QAbstractTableModel>

/**
 * \brief Implementation of the QAbstractTableModel for ultrasound risk zones.
 * This class manages the data model for the QmitkUSZoneManagementWidget. It provides
 * consistency between the table in QmitkUSZoneManagementWidget and the DataStorage.
 */
class QmitkUSZonesDataModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  typedef std::vector<mitk::DataNode::Pointer> DataNodeVector;

  explicit QmitkUSZonesDataModel(QObject *parent = 0);
  virtual ~QmitkUSZonesDataModel();

  /**
   * \brief Set data storage and base node for the zone nodes of this model.
   * The node event listeners will only recognize nodes which are children
   * of the given base node.
   *
   * \param dataStorage DataStorage where the zone nodes will be contained
   * \param baseNode DataNode which is source node of the zone nodes
   */
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::DataNode::Pointer baseNode);

  void AddNode(const mitk::DataNode*);
  void RemoveNode(const mitk::DataNode*);
  void ChangeNode(const mitk::DataNode*);

  /** \brief Return number of rows of the model. */
  virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

  /** \brief Return number of columns (3) of the model. */
  virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;

  /** \brief Return names for the columns, numbers for the rows and invalid for DisplayRole. */
  virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  /** \brief Return selectable and enabled for column 1 (size); selectable, enabled and editable for every other column. */
  virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;

  /** \brief Return model data of the selected cell. */
  virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

  /** \brief Set model data for the selected cell. */
  virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

  /** \brief Insert empty rows into the model (creates zone nodes). */
  virtual bool insertRows ( int row, int count, const QModelIndex& parent = QModelIndex() );

  /** \brief Remove given rows from the model. */
  virtual bool removeRows ( int row, int count, const QModelIndex& parent = QModelIndex() );

  /** \brief Remove given rows from the model.
   *  \param removeFromDataStorage zone nodes are removed from the data storage too, if this is set to true
   */
  virtual bool removeRows ( int row, int count, const QModelIndex& parent, bool removeFromDataStorage );

protected:
  DataNodeVector             m_ZoneNodes;

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer    m_BaseNode;

private:
  mitk::MessageDelegate1<QmitkUSZonesDataModel, const mitk::DataNode*> m_ListenerAddNode;
  mitk::MessageDelegate1<QmitkUSZonesDataModel, const mitk::DataNode*> m_ListenerChangeNode;
  mitk::MessageDelegate1<QmitkUSZonesDataModel, const mitk::DataNode*> m_ListenerRemoveNode;
};

#endif // QMITKUSZONESDATAMODEL_H
