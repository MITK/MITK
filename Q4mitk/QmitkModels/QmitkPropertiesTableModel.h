/*=========================================================================

 Program:   MITK
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 14921 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QmitkPropertiesTableModel_h
#define QmitkPropertiesTableModel_h

/// Own includes.
#include "mitkDataTreeNode.h"

/// Toolkit includes.
#include <QAbstractTableModel>

/// Forward declarations.

/// 
/// \class QmitkPropertiesTableModel
/// \brief A table model for the properties of a node.
///
/// \see QmitkPropertyDelegate
class QMITK_EXPORT QmitkPropertiesTableModel : public QAbstractTableModel
{
public:

  ///
  /// Constructs a new QmitkDataStorageTableModel 
  /// and sets the DataTreeNode for this TableModel.
  QmitkPropertiesTableModel(mitk::DataTreeNode::Pointer _Node=0
    , QObject* parent = 0);

  ///
  /// Standard dtor. Nothing to do here.
  virtual ~QmitkPropertiesTableModel();

  ///
  /// Sets the node. Resets the whole model. If _Node is NULL the model is empty.
  ///
  void setNode(mitk::DataTreeNode::Pointer _Node);

  ///
  /// Get the node.
  /// 
  mitk::DataTreeNode::Pointer getNode() const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  Qt::ItemFlags flags(const QModelIndex& index) const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  ///
  /// Overridden from QAbstractTableModel. Sets data at index for given role.
  ///
  bool setData(const QModelIndex &index, const QVariant &value,
    int role);

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the flags what can be done with the items (view, edit, ...)
  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  ///
  /// Overwritten from QAbstractTableModel. Returns the number of columns. That is usually two in this model:
  /// the properties name and its value.
  int columnCount(const QModelIndex &parent) const;

protected:
  /// 
  /// Holds the node the properties belong to.s
  mitk::DataTreeNode::Pointer m_Node;
};

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
