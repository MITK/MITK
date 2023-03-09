/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageTreeModel_h
#define QmitkDataStorageTreeModel_h

#include <MitkQtWidgetsExports.h>

#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>
#include <mitkWeakPointer.h>

#include <QAbstractListModel>

#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

#include <QList>
#include <string>
#include <vector>

class QmitkDataStorageTreeModelInternalItem;

/** \ingroup QmitkModule
 @warning This class causes invalid point exception when used with invalid QModelIndex instances.
 The index validation is not sufficient. This may cause unspecific crashes in situation where
 this class is used multiple times or with multiple selection models. See https://phabricator.mitk.org/T24348
 for more information.
*/
class MITKQTWIDGETS_EXPORT QmitkDataStorageTreeModel : public QAbstractItemModel
{
  Q_OBJECT

  //# CONSTANTS,TYPEDEFS
public:
  static const std::string COLUMN_NAME;
  static const std::string COLUMN_TYPE;
  static const std::string COLUMN_VISIBILITY;

  //# CTORS,DTOR
public:
  QmitkDataStorageTreeModel(mitk::DataStorage *_DataStorage, bool _PlaceNewNodesOnTop = false, QObject *parent = nullptr);
  ~QmitkDataStorageTreeModel() override;

  //# GETTER
public:
  ///
  /// Get node at a specific model index.
  /// This function is used to get a node from a QModelIndex
  ///
  mitk::DataNode::Pointer GetNode(const QModelIndex &index) const;
  ///
  /// Returns a copy of the node-vector that is shown by this model
  ///
  virtual QList<mitk::DataNode::Pointer> GetNodeSet() const;
  ///
  /// Get the DataStorage.
  ///
  const mitk::DataStorage::Pointer GetDataStorage() const;

  ///
  /// Get the top placement flag
  ///
  bool GetPlaceNewNodesOnTopFlag() { return m_PlaceNewNodesOnTop; }
  ///
  /// Set the top placement flag
  ///
  void SetPlaceNewNodesOnTop(bool _PlaceNewNodesOnTop);

  //# (Re-)implemented from QAbstractItemModel
  //# Read model
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  //# hierarchical model
  ///
  /// called whenever the model or the view needs to create a QModelIndex for a particular
  /// child item (or a top-level item if parent is an invalid QModelIndex)
  ///
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  //# editable model
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
  bool dropMimeData(
    const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
  Qt::DropActions supportedDropActions() const override;
  Qt::DropActions supportedDragActions() const override;
  QStringList mimeTypes() const override;
  QMimeData *mimeData(const QModelIndexList &indexes) const override;

  static QMimeData *mimeDataFromModelIndexList(const QModelIndexList &indexes);

  //# End of QAbstractItemModel

  //# SETTER
public:
  ///
  /// Sets the DataStorage. The whole model will be resetted.
  ///
  void SetDataStorage(mitk::DataStorage *_DataStorage);
  ///
  /// Notify that the DataStorage was deleted. The whole model will be resetted.
  ///
  void SetDataStorageDeleted();
  ///
  /// Adds a node to this model.
  /// If a predicate is set (not null) the node will be checked against it.The node has to have a data object (no one
  /// wants to see empty nodes).
  ///
  virtual void AddNode(const mitk::DataNode *node);
  ///
  /// Removes a node from this model. Also removes any event listener from the node.
  ///
  virtual void RemoveNode(const mitk::DataNode *node);
  ///
  /// Sets a node to modfified. Called by the DataStorage
  ///
  virtual void SetNodeModified(const mitk::DataNode *node);

  ///
  /// \return an index for the given datatreenode in the tree. If the node is not found
  ///
  QModelIndex GetIndex(const mitk::DataNode *) const;

  /// Set whether to allow hierarchy changes by dragging and dropping
  void SetAllowHierarchyChange(bool allowHierarchyChange);

signals:

 void nodeVisibilityChanged();

  //# MISC
protected:

  using TreeItem = QmitkDataStorageTreeModelInternalItem;

  QList<TreeItem *> ToTreeItemPtrList(const QMimeData *mimeData);
  QList<TreeItem *> ToTreeItemPtrList(const QByteArray &ba);

  ///
  /// Adjusts the LayerProperty according to the nodes position
  ///
  void AdjustLayerProperty();
  ///
  /// invoked after m_DataStorage or m_Predicate changed
  ///
  TreeItem *TreeItemFromIndex(const QModelIndex &index) const;
  ///
  /// Gives a ModelIndex for the Tree Item
  ///
  QModelIndex IndexFromTreeItem(TreeItem *) const;
  ///
  /// Returns the first element in the nodes sources list (if available) or 0
  ///
  mitk::DataNode *GetParentNode(const mitk::DataNode *node) const;
  ///
  /// Adds all Childs in parent to vec. Before a child is added the function is called recursively
  ///
  void TreeToVector(TreeItem *parent, std::vector<TreeItem *> &vec) const;
  ///
  /// Adds all Childs in parent to vec. Before a child is added the function is called recursively
  ///
  void TreeToNodeSet(TreeItem *parent, QList<mitk::DataNode::Pointer> &vec) const;
  ///
  /// Update Tree Model
  ///
  void Update();

  //# ATTRIBUTES
protected:
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::Pointer m_Predicate;
  bool m_PlaceNewNodesOnTop;
  TreeItem *m_Root;

  /// Flag to block the data storage events if nodes are added/removed by this class.
  bool m_BlockDataStorageEvents;

  /// This decides whether or not it is allowed to assign a different parent to a node
  /// If it is false, it is not possible to change the hierarchy of nodes by dragging
  /// and dropping.
  /// If it is true, dragging nodes on another node will replace all of their parents
  /// with that one.
  bool m_AllowHierarchyChange;

private:
  void AddNodeInternal(const mitk::DataNode *);
  void RemoveNodeInternal(const mitk::DataNode *);
  ///
  /// Checks if dicom properties patient name, study names and series name exists
  ///
  bool DicomPropertiesExists(const mitk::DataNode &) const;

  unsigned long m_DataStorageDeletedTag;
};

#endif
