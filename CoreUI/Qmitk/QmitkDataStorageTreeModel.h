/*=========================================================================

 Program:   MITK
 Language:  C++
 Date:      $Date: 2008-08-13 16:56:36 +0200 (Mi, 13 Aug 2008) $
 Version:   $Revision: 14972 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QMITKDATASTORAGETREEMODEL_H_
#define QMITKDATASTORAGETREEMODEL_H_

#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>
#include <mitkWeakPointer.h>

#include <QAbstractListModel>

#include "QmitkEnums.h"
#include "QmitkCustomVariants.h"

#include <vector>
#include <string>
#include <QList>

class QMITK_EXPORT QmitkDataStorageTreeModel : public QAbstractItemModel
{
//# CONSTANTS,TYPEDEFS
public:
  static const std::string COLUMN_NAME;
  static const std::string COLUMN_TYPE;
  static const std::string COLUMN_VISIBILITY;

//# CTORS,DTOR
public:
  QmitkDataStorageTreeModel(mitk::DataStorage* _DataStorage, QObject* parent = 0);
  ~QmitkDataStorageTreeModel();

//# GETTER
public:
  ///
  /// Get node at a specific model index.
  /// This function is used to get a node from a QModelIndex
  ///
  mitk::DataTreeNode::Pointer GetNode(const QModelIndex &index) const;
  ///
  /// Returns a copy of the node-vector that is shown by this model
  ///
  virtual std::vector<mitk::DataTreeNode*> GetNodeSet() const;
  ///
  /// Get the DataStorage.
  /// 
  const mitk::DataStorage::Pointer GetDataStorage() const;

  //# (Re-)implemented from QAbstractItemModel
  //# Read model
  Qt::ItemFlags flags(const QModelIndex& index) const;  
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
  int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
  //# hierarchical model
  ///
  /// called whenever the model or the view needs to create a QModelIndex for a particular
  /// child item (or a top-level item if parent is an invalid QModelIndex)
  ///
  QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
  QModelIndex parent ( const QModelIndex & index ) const;
  //# editable model
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
  bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
  bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
  Qt::DropActions supportedDropActions() const;
  QMimeData * mimeData(const QModelIndexList & indexes) const;

  //# End of QAbstractItemModel

//# SETTER
public:
  ///
  /// Sets the DataStorage. The whole model will be resetted.
  ///
  void SetDataStorage(mitk::DataStorage* _DataStorage);
  ///
  /// Notify that the DataStorage was deleted. The whole model will be resetted.
  ///
  void SetDataStorageDeleted(const itk::Object* _DataStorage);
  ///
  /// Adds a node to this model. 
  /// If a predicate is set (not null) the node will be checked against it.The node has to have a data object (no one wants to see empty nodes).
  ///
  virtual void AddNode(const mitk::DataTreeNode* node);
  ///
  /// Removes a node from this model. Also removes any event listener from the node.
  ///
  virtual void RemoveNode(const mitk::DataTreeNode* node);
  ///
  /// Sets a node to modfified. Called by the DataStorage
  ///
  virtual void SetNodeModified(const mitk::DataTreeNode* node);

  ///
  /// \return an index for the given datatreenode in the tree. If the node is not found
  ///
  QModelIndex GetIndex(const mitk::DataTreeNode*) const;

//# MISC
protected:
  ///
  /// Helper class to represent a tree structure of DataTreeNodes
  ///
  class TreeItem
  {
  public:
    ///
    /// Constructs a new TreeItem with the given DataTreeNode (must not be 0)
    ///
    TreeItem(mitk::DataTreeNode* _DataTreeNode, TreeItem* _Parent=0);
    ///
    /// Removes itself as child from its parent-> Does not delete its children
    /// \sa Delete()
    ///
    virtual ~TreeItem();
    ///
    /// Find the index of an item
    ///
    int IndexOfChild(const TreeItem* item) const;
    ///
    /// \child The child at pos index or 0 if it not exists
    ///
    TreeItem* GetChild(int index) const;
    ///
    /// Find the TreeItem containing a special tree node (recursive tree function)
    ///
    TreeItem* Find( const mitk::DataTreeNode* _DataTreeNode) const;
    ///
    /// Get the amount of children
    ///
    int GetChildCount() const;
    ///
    /// \return the index of this node in its parent list
    ///
    int GetIndex() const;
    ///
    /// \return the parent of this tree item
    ///
    TreeItem* GetParent() const;
    ///
    /// Return the DataTreeNode associated with this node
    ///
    mitk::DataTreeNode::Pointer GetDataTreeNode() const;
    ///
    /// Get all children as vector
    ///
    std::vector<TreeItem*> GetChildren() const;

    ///
    /// add another item as a child of this (only if not already in that list)
    ///
    void AddChild( TreeItem* item);
    ///
    /// remove another item as child from this
    ///
    void RemoveChild( TreeItem* item);
    ///
    /// inserts a child at the given position. if pos is not in range
    /// the element is added at the end
    ///
    void InsertChild( TreeItem* item, int index=-1 );
    /// Sets the parent on the treeitem
    void SetParent(TreeItem* _Parent);
    ///
    /// Deletes the whole tree branch
    ///
    void Delete();
  protected:
  protected:
    TreeItem* m_Parent;
    std::vector<TreeItem*> m_Children;
    mitk::DataTreeNode::Pointer m_DataTreeNode;
  };
  ///
  /// Adjusts the LayerProperty according to the nodes position
  ///
  void AdjustLayerProperty();
  ///
  /// invoked after m_DataStorage or m_Predicate changed
  ///  
  TreeItem* TreeItemFromIndex(const QModelIndex &index) const;
  ///
  /// Gives a ModelIndex for the Tree Item
  ///
  QModelIndex IndexFromTreeItem(TreeItem*) const;
  ///
  /// Returns the first element in the nodes sources list (if available) or 0
  ///
  mitk::DataTreeNode* GetParentNode(const mitk::DataTreeNode* node) const;
  ///
  /// Adds all Childs in parent to vec. Before a child is added the function is called recursively
  ///
  void TreeToVector(TreeItem* parent, std::vector<TreeItem*>& vec) const;
  ///
  /// Adds all Childs in parent to vec. Before a child is added the function is called recursively
  ///
  void TreeToNodeSet(TreeItem* parent, std::vector<mitk::DataTreeNode*>& vec) const;
  //# ATTRIBUTES
protected:
  mitk::WeakPointer<mitk::DataStorage> m_DataStorage;
  mitk::NodePredicateBase::Pointer m_Predicate;
  TreeItem* m_Root;
};

#endif /* QMITKDATASTORAGETREEMODEL_H_ */
