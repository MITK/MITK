/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataStorageSelection_h
#define mitkDataStorageSelection_h

#include <MitkDataTypesExtExports.h>

#include <mitkCommon.h>
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

#include <set>

namespace mitk
{
  class BaseProperty;
  class PropertyList;

  /**
   * \brief Manages a selection of DataNode objects from a DataStorage.
   *
   * Observes a DataStorage for node additions and removals, optionally
   * filtering by a NodePredicateBase. Fires events (NodeChanged, NodeAdded,
   * NodeRemoved, PropertyChanged) when the selection or its properties change.
   *
   * Nodes can be automatically added when they appear in the DataStorage
   * (if AutoAddNodes is enabled), or manually added/removed.
   *
   * \sa DataStorage, NodePredicateBase
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT DataStorageSelection : public itk::Object
  {
  public:
    typedef std::vector<mitk::DataNode *> Nodes;
    typedef Message1<const mitk::DataNode *> DataNodeEvent;

    /** \brief Event fired when a node in the selection is modified. */
    DataNodeEvent NodeChanged;
    /** \brief Event fired when a node is added to the selection. */
    DataNodeEvent NodeAdded;
    /** \brief Event fired when a node is removed from the selection. */
    DataNodeEvent NodeRemoved;
    /** \brief Event fired when a property on a selected node changes. */
    Message2<const mitk::DataNode *, const mitk::BaseProperty *> PropertyChanged;

    mitkClassMacroItkParent(DataStorageSelection, itk::Object);

    mitkNewMacro2Param(DataStorageSelection, DataStorage *, bool);
    mitkNewMacro3Param(DataStorageSelection, DataStorage *, NodePredicateBase *, bool);

  protected:
    DataStorageSelection(mitk::DataStorage *_DataStorage, bool _AutoAddNodes);
    DataStorageSelection(mitk::DataStorage *_DataStorage, mitk::NodePredicateBase *_Predicate, bool _AutoAddNodes);

  public:
    ~DataStorageSelection() override;

    /**
     * \brief Get the associated DataStorage.
     * \return Smart pointer to the DataStorage, or nullptr.
     */
    mitk::DataStorage::Pointer GetDataStorage() const;

    /**
     * \brief Get the predicate used for filtering nodes.
     * \return Smart pointer to the predicate, or nullptr if none is set.
     */
    mitk::NodePredicateBase::Pointer GetPredicate() const;

    /**
     * \brief Returns the number of nodes in this selection.
     * \return The node count.
     */
    unsigned int GetSize() const;

    /**
     * \brief Get a node at a specific index.
     * \param[in] index The zero-based index.
     * \return Smart pointer to the node, or nullptr if index is out of range.
     */
    mitk::DataNode::Pointer GetNode(unsigned int index) const;

    /**
     * \brief Returns the first node in the selection.
     *
     * Equivalent to calling GetNode(0).
     *
     * \return Smart pointer to the first node, or nullptr if empty.
     */
    mitk::DataNode::Pointer GetNode() const;

    /**
     * \brief Returns a copy of the internal node vector.
     * \return A vector of raw DataNode pointers.
     */
    std::vector<mitk::DataNode *> GetNodes() const;

    /**
     * \brief Query whether nodes are automatically added from the DataStorage.
     * \return true if auto-add is enabled.
     */
    bool DoesAutoAddNodes() const;

  public:
    /**
     * \brief Remove all nodes and set a raw pointer as the sole element.
     * \param[in] node The node to set.
     * \return Reference to this selection.
     */
    DataStorageSelection &operator=(mitk::DataNode *node);

    /**
     * \brief Remove all nodes and set a smart pointer as the sole element.
     * \param[in] node The node to set.
     * \return Reference to this selection.
     */
    DataStorageSelection &operator=(mitk::DataNode::Pointer node);

    /**
     * \brief Set or change the associated DataStorage.
     * \param[in] _DataStorage Pointer to the new DataStorage, or nullptr to detach.
     */
    virtual void SetDataStorage(mitk::DataStorage *_DataStorage);

    /**
     * \brief Set or change the filter predicate.
     *
     * Ownership of the predicate is taken by this class.
     *
     * \param[in] _Predicate The new predicate, or nullptr to accept all nodes.
     */
    virtual void SetPredicate(mitk::NodePredicateBase *_Predicate);

    /**
     * \brief Add a node to the selection if it is not already present.
     * \param[in] node The node to add.
     */
    virtual void AddNode(const mitk::DataNode *node);

    /**
     * \brief Remove a node from the selection.
     * \param[in] node The node to remove.
     */
    virtual void RemoveNode(const mitk::DataNode *node);

    /**
     * \brief Remove all nodes from the selection.
     */
    virtual void RemoveAllNodes();

    /**
     * \brief Callback invoked when an observed ITK object is deleted or modified.
     * \param[in] caller The object that triggered the event.
     * \param[in] event The event descriptor.
     */
    virtual void ObjectChanged(const itk::Object *caller, const itk::EventObject &event);

  protected:
    ///
    /// Find a node in the list by the given prop
    ///
    mitk::DataNode::Pointer FindNode(const mitk::BaseProperty *prop) const;
    ///
    /// Find a node in the list by the given proplist
    ///
    mitk::DataNode::Pointer FindNode(const mitk::PropertyList *propList) const;
    ///
    /// Removes all nodes and fill the vector again
    ///
    void Reset();
    ///
    /// If a node is already in this list, all listeners will be removed
    ///
    void RemoveListener(mitk::DataNode *node);
    ///
    /// Adds listeners for modified/delete event, for the propertylists
    /// modified/delete and for the modified/delete event of each property
    /// and stores listener tags
    ///
    void AddListener(mitk::DataNode *node);

  protected:
    ///
    /// Pointer to the DataStorage from which the nodes are selected
    ///
    mitk::DataStorage *m_DataStorage;
    ///
    /// DeleteTag for the DataStorage
    ///
    unsigned int m_DataStorageDeletedTag;
    ///
    /// Holds the predicate that defines this SubSet of Nodes. If m_Predicate
    /// is nullptr all Nodes will be selected.
    ///
    mitk::NodePredicateBase::Pointer m_Predicate;
    ///
    /// Holds all selected Nodes.
    ///
    std::vector<mitk::DataNode *> m_Nodes;
    ///
    /// \brief Maps a node to a modified observer tag.
    ///
    std::map<mitk::DataNode *, unsigned long> m_NodeModifiedObserverTags;
    ///
    /// \brief Maps a propertylist to a modified observer tag.
    ///
    std::map<mitk::PropertyList *, unsigned long> m_PropertyListModifiedObserverTags;
    ///
    /// \brief Maps a propertylist to a delete observer tag.
    ///
    std::map<mitk::PropertyList *, unsigned long> m_PropertyListDeletedObserverTags;
    ///
    /// \brief Maps a property to  a modified observer tag.
    ///
    std::map<mitk::BaseProperty *, unsigned long> m_PropertyModifiedObserverTags;
    ///
    /// \brief Maps a property to a delete observer tag.
    ///
    std::map<mitk::BaseProperty *, unsigned long> m_PropertyDeletedObserverTags;
    ///
    /// If set to true no event processing will be performed
    ///
    bool m_SelfCall;
    ///
    /// Saves if new nodes are automatically added to this selection
    ///
    bool m_AutoAddNodes;
  };
}

#endif
