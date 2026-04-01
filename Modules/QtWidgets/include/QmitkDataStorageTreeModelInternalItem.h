/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataStorageTreeModelInternalItem_h
#define QmitkDataStorageTreeModelInternalItem_h

#include <MitkQtWidgetsExports.h>

#include <mitkDataNode.h>

#include <string>
#include <vector>

  /**
   * \brief Helper class representing a node in a tree structure of DataNodes.
   *
   * This class is used internally by QmitkDataStorageTreeModel and
   * QmitkDataStorageSimpleTreeModel to build an in-memory tree hierarchy
   * that mirrors the parent-child relationships in the DataStorage.
   * Each item holds a weak pointer to a mitk::DataNode and references
   * to its parent and children.
   *
   * \sa QmitkDataStorageTreeModel
   * \sa QmitkDataStorageSimpleTreeModel
   */
  class MITKQTWIDGETS_EXPORT QmitkDataStorageTreeModelInternalItem
  {
  public:
    /**
     * \brief Constructs a tree item for the given DataNode.
     * \param[in] _DataNode The data node to associate with this item. Must not be nullptr.
     * \param[in] _Parent   Optional parent item. Defaults to nullptr (root item).
     */
    QmitkDataStorageTreeModelInternalItem(mitk::DataNode *_DataNode, QmitkDataStorageTreeModelInternalItem *_Parent = nullptr);

    /**
     * \brief Destructor. Removes this item from its parent's child list.
     * \note Does not delete child items. Use Delete() to recursively remove an entire branch.
     * \sa Delete
     */
    virtual ~QmitkDataStorageTreeModelInternalItem();

    /**
     * \brief Returns the index of the given child item.
     * \param[in] item The child item to find.
     * \return The zero-based index, or -1 if the item is not a child.
     */
    int IndexOfChild(const QmitkDataStorageTreeModelInternalItem *item) const;

    /**
     * \brief Returns the child item at the given index.
     * \param[in] index The zero-based child index.
     * \return Pointer to the child item, or nullptr if the index is out of range.
     */
    QmitkDataStorageTreeModelInternalItem *GetChild(int index) const;

    /**
     * \brief Recursively searches for the item holding the given DataNode.
     * \param[in] _DataNode The data node to search for.
     * \return Pointer to the matching item, or nullptr if not found.
     */
    QmitkDataStorageTreeModelInternalItem *Find(const mitk::DataNode *_DataNode) const;

    /**
     * \brief Returns the number of child items.
     * \return The child count.
     */
    int GetChildCount() const;

    /**
     * \brief Returns the index of this item within its parent's child list.
     * \return The zero-based index, or -1 if this item has no parent.
     */
    int GetIndex() const;

    /**
     * \brief Returns the parent item.
     * \return Pointer to the parent item, or nullptr for root items.
     */
    QmitkDataStorageTreeModelInternalItem *GetParent() const;

    /**
     * \brief Returns the DataNode associated with this tree item.
     * \return A smart pointer to the data node.
     */
    mitk::DataNode::Pointer GetDataNode() const;

    /**
     * \brief Returns all child items as a vector.
     * \return A vector of pointers to child items.
     */
    std::vector<QmitkDataStorageTreeModelInternalItem *> GetChildren() const;

    /**
     * \brief Adds the given item as a child (if not already present).
     * \param[in] item The item to add as a child.
     */
    void AddChild(QmitkDataStorageTreeModelInternalItem *item);

    /**
     * \brief Removes the given item from this item's child list.
     * \param[in] item The child item to remove.
     */
    void RemoveChild(QmitkDataStorageTreeModelInternalItem *item);

    /**
     * \brief Inserts a child item at the given position.
     *
     * If \p index is out of range (or -1), the item is appended at the end.
     *
     * \param[in] item  The item to insert.
     * \param[in] index The position at which to insert. Defaults to -1 (append).
     */
    void InsertChild(QmitkDataStorageTreeModelInternalItem *item, int index = -1);

    /**
     * \brief Sets the parent of this item.
     * \param[in] _Parent The new parent item.
     */
    void SetParent(QmitkDataStorageTreeModelInternalItem *_Parent);

    /**
     * \brief Recursively deletes this item and all its children.
     */
    void Delete();

  protected:
    QmitkDataStorageTreeModelInternalItem *m_Parent;
    std::vector<QmitkDataStorageTreeModelInternalItem *> m_Children;
    mitk::WeakPointer<mitk::DataNode> m_DataNode;
  };

#endif
