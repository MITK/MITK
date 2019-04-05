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

#ifndef QMITKDATASTORAGETREEMODELINTERNALITEM_H_
#define QMITKDATASTORAGETREEMODELINTERNALITEM_H_

#include <MitkQtWidgetsExports.h>

#include <mitkDataNode.h>

#include <string>
#include <vector>

  ///
  /// Helper class to represent a tree structure of DataNodes
  ///
  class MITKQTWIDGETS_EXPORT QmitkDataStorageTreeModelInternalItem
  {
  public:
    ///
    /// Constructs a new QmitkDataStorageTreeModelInternalItem with the given DataNode (must not be 0)
    ///
    QmitkDataStorageTreeModelInternalItem(mitk::DataNode *_DataNode, QmitkDataStorageTreeModelInternalItem *_Parent = 0);
    ///
    /// Removes itself as child from its parent-> Does not delete its children
    /// \sa Delete()
    ///
    virtual ~QmitkDataStorageTreeModelInternalItem();
    ///
    /// Find the index of an item
    ///
    int IndexOfChild(const QmitkDataStorageTreeModelInternalItem *item) const;
    ///
    /// \return The child at pos index or 0 if it not exists
    ///
    QmitkDataStorageTreeModelInternalItem *GetChild(int index) const;
    ///
    /// Find the QmitkDataStorageTreeModelInternalItem containing a special tree node (recursive tree function)
    ///
    QmitkDataStorageTreeModelInternalItem *Find(const mitk::DataNode *_DataNode) const;
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
    QmitkDataStorageTreeModelInternalItem *GetParent() const;
    ///
    /// Return the DataNode associated with this node
    ///
    mitk::DataNode::Pointer GetDataNode() const;
    ///
    /// Get all children as vector
    ///
    std::vector<QmitkDataStorageTreeModelInternalItem *> GetChildren() const;

    ///
    /// add another item as a child of this (only if not already in that list)
    ///
    void AddChild(QmitkDataStorageTreeModelInternalItem *item);
    ///
    /// remove another item as child from this
    ///
    void RemoveChild(QmitkDataStorageTreeModelInternalItem *item);
    ///
    /// inserts a child at the given position. if pos is not in range
    /// the element is added at the end
    ///
    void InsertChild(QmitkDataStorageTreeModelInternalItem *item, int index = -1);
    /// Sets the parent on the QmitkDataStorageTreeModelInternalItem
    void SetParent(QmitkDataStorageTreeModelInternalItem *_Parent);
    ///
    /// Deletes the whole tree branch
    ///
    void Delete();

  protected:
    QmitkDataStorageTreeModelInternalItem *m_Parent;
    std::vector<QmitkDataStorageTreeModelInternalItem *> m_Children;
    mitk::WeakPointer<mitk::DataNode> m_DataNode;
  };

#endif /* QMITKDATASTORAGETREEMODEL_H_ */
