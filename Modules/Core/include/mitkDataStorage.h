/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataStorage_h
#define mitkDataStorage_h

#include <itkObject.h>
#include <itkVectorContainer.h>
#include <mitkDataNode.h>
#include <mitkGeometry3D.h>
#include <mitkMessage.h>
#include <MitkCoreExports.h>
#include <map>
#include <mutex>

namespace mitk
{
  class NodePredicateBase;
  class DataNode;
  class BaseRenderer;

  /**
   * \brief Abstract data management class that organizes DataNode objects in a directed acyclic graph.
   *
   * The DataStorage provides storage and retrieval functionality for DataNode objects.
   * It tracks 'was created by' relationships by associating each node with a set of
   * source (parent) nodes from which it was derived. This forms a non-cyclical directed
   * graph data structure.
   *
   * The DataStorage emits events when nodes are added (AddNodeEvent), removed (RemoveNodeEvent),
   * modified (ChangedNodeEvent), or deleted (DeleteNodeEvent). It also provides methods
   * for querying nodes by predicates (GetSubset), computing bounding geometries, and
   * traversing source/derivation relationships.
   *
   * This is an abstract base class. Use StandaloneDataStorage for a concrete implementation.
   *
   * \ingroup DataStorage
   * \sa StandaloneDataStorage, DataNode, NodePredicateBase
   */
  class MITKCORE_EXPORT DataStorage : public itk::Object
  {
  public:
    mitkClassMacroItkParent(DataStorage, itk::Object);

    /**
     * \brief Container type used as a result set of query operations such as GetSubset().
     *
     * Holds smart pointers to DataNode objects.
     */
    typedef itk::VectorContainer<unsigned int, DataNode::Pointer> SetOfObjects;

    /**
     * \brief Container type for const node pointers, used for input parameters where nodes are read-only.
     *
     * Callers that hold only DataNode::ConstPointer can use this container to pass nodes to DataStorage
     * operations without needing a const_cast at the call site. Internally, the DataStorage converts
     * to mutable pointers at its ownership boundary, as it owns all nodes as mutable objects.
     */
    typedef itk::VectorContainer<unsigned int, DataNode::ConstPointer> ConstSetOfObjects;

    /**
     * \brief Add a DataNode to the storage, optionally with parent (source) nodes.
     *
     * The node is added to the internal graph structure. If \a parents is provided,
     * 'was created from' relationships are established between the node and its parents.
     * After a successful addition, AddNodeEvent is emitted.
     *
     * \param[in] node     The DataNode to add. Must not be nullptr.
     * \param[in] parents  Optional set of parent nodes. May be nullptr.
     * \throw std::invalid_argument if \a node is nullptr or already exists in the DataStorage.
     * \sa Remove, AddNodeEvent
     */
    virtual void Add(DataNode *node, const DataStorage::SetOfObjects *parents = nullptr) = 0;

    /**
     * \brief Convenience method to add a node with a single parent.
     *
     * \param[in] node    The DataNode to add. Must not be nullptr.
     * \param[in] parent  The parent node. May be nullptr (equivalent to adding without parents).
     * \throw std::invalid_argument if \a node is nullptr or already exists in the DataStorage.
     * \sa Add(DataNode*, const SetOfObjects*)
     */
    void Add(DataNode *node, DataNode *parent);

    /**
     * \brief Convenience overload for callers that hold only const node pointers for the parents.
     *
     * Converts the ConstSetOfObjects to a SetOfObjects internally and delegates to the
     * virtual Add() method.
     *
     * \param[in] node     The DataNode to add. Must not be nullptr.
     * \param[in] parents  Optional set of const parent node pointers. May be nullptr.
     * \throw std::invalid_argument if \a node is nullptr or already exists in the DataStorage.
     * \sa Add(DataNode*, const SetOfObjects*)
     */
    void Add(DataNode *node, const ConstSetOfObjects *parents);

    /**
     * \brief Remove a single node from the DataStorage.
     *
     * The RemoveNodeEvent is emitted before the node is actually removed.
     *
     * \param[in] node  The node to remove. If nullptr, the call is ignored.
     * \sa Add, RemoveNodeEvent
     */
    virtual void Remove(const DataNode *node) = 0;

    /**
     * \brief Check whether a node exists in the DataStorage.
     *
     * \param[in] node  The node to check.
     * \return \a true if the node is present in the DataStorage.
     */
    virtual bool Exists(const DataNode *node) const = 0;

    /**
     * \brief Remove a set of nodes from the DataStorage.
     *
     * If \a nodes is nullptr, this method does nothing.
     *
     * \param[in] nodes  The set of nodes to remove.
     * \sa Remove(const DataNode*)
     */
    void Remove(const DataStorage::SetOfObjects *nodes);

    /**
     * \brief Remove a set of const-pointer nodes from the DataStorage.
     *
     * If \a nodes is nullptr, this method does nothing.
     *
     * \param[in] nodes  The set of const node pointers to remove.
     * \sa Remove(const DataNode*)
     */
    void Remove(const ConstSetOfObjects *nodes);

    /**
     * \brief Return all nodes that satisfy the given predicate condition.
     *
     * Conditions can test for data type, properties, source relationships, or combinations
     * thereof (AND, OR, NOT) using the Composite Design Pattern (see NodePredicateBase).
     *
     * To retrieve all nodes without filtering, use GetAll().
     *
     * \param[in] condition  The predicate to evaluate. If nullptr, all nodes are returned.
     * \return A set of DataNode smart pointers that fulfill the condition.
     * \sa GetAll, GetNode, NodePredicateBase
     */
    SetOfObjects::ConstPointer GetSubset(const NodePredicateBase *condition) const;

    /**
     * \brief Return the source (parent) nodes of a given node.
     *
     * \param[in] node              The node whose sources are queried.
     * \param[in] condition         Optional predicate to filter results.
     * \param[in] onlyDirectSources If true (default), only direct parents are returned.
     *                              If false, the entire transitive source chain is traversed.
     * \return A set of source DataNodes, possibly filtered by \a condition.
     * \throw std::invalid_argument if \a node is nullptr.
     * \sa GetDerivations
     */
    virtual SetOfObjects::ConstPointer GetSources(const DataNode *node,
                                                  const NodePredicateBase *condition = nullptr,
                                                  bool onlyDirectSources = true) const = 0;

    /**
     * \brief Return the derived (child) nodes of a given node.
     *
     * Returns nodes that were created from (derived from) the given \a node.
     *
     * \param[in] node                  The node whose derivations are queried.
     * \param[in] condition             Optional predicate to filter results.
     * \param[in] onlyDirectDerivations If true (default), only direct children are returned.
     *                                  If false, the entire transitive derivation chain is traversed.
     * \return A set of derived DataNodes, possibly filtered by \a condition.
     * \throw std::invalid_argument if \a node is nullptr.
     * \sa GetSources
     */
    virtual SetOfObjects::ConstPointer GetDerivations(const DataNode *node,
                                                      const NodePredicateBase *condition = nullptr,
                                                      bool onlyDirectDerivations = true) const = 0;

    /**
     * \brief Return all nodes managed by this DataStorage.
     *
     * \return A set containing all DataNodes currently in the storage.
     * \sa GetSubset
     */
    virtual SetOfObjects::ConstPointer GetAll() const = 0;

    /**
     * \brief Convenience method to get the first node that matches the predicate condition.
     *
     * \param[in] condition  The predicate to evaluate. If nullptr, returns nullptr.
     * \return The first matching DataNode, or nullptr if none matches.
     * \sa GetSubset
     */
    DataNode *GetNode(const NodePredicateBase *condition = nullptr) const;

    /**
     * \brief Convenience method to get the first node with a given name.
     *
     * Searches for a node whose "name" StringProperty matches \a name.
     *
     * \param[in] name  The node name to search for.
     * \return The first matching DataNode, or nullptr if not found or \a name is nullptr.
     * \sa GetNamedDerivedNode
     */
    DataNode *GetNamedNode(const char *name) const;

    /**
     * \brief Convenience method to get the first node with a given name (std::string overload).
     *
     * \param[in] name  The node name to search for.
     * \return The first matching DataNode, or nullptr if not found.
     * \sa GetNamedNode(const char*)
     */
    DataNode *GetNamedNode(const std::string& name) const { return this->GetNamedNode(name.c_str()); }

    /**
     * \brief Convenience method to get the first node with a given name that is derived from a source node.
     *
     * \param[in] name                  The node name to search for.
     * \param[in] sourceNode            The parent node to search derivations of.
     * \param[in] onlyDirectDerivations If true, only direct derivations are searched.
     * \return The first matching DataNode, or nullptr if not found or \a name is nullptr.
     * \sa GetNamedNode, GetDerivations
     */
    DataNode *GetNamedDerivedNode(const char *name,
                                  const DataNode *sourceNode,
                                  bool onlyDirectDerivations = true) const;

    /**
     * \brief Convenience method to get the first data object of a given type with a given name.
     *
     * Searches for a named node and dynamic_casts its data to the requested type.
     *
     * \tparam DataType  The expected data type (must inherit from BaseData).
     * \param[in] name  The node name to search for.
     * \return Pointer to the data object cast to DataType, or nullptr if not found.
     * \sa GetNamedNode
     */
    template <class DataType>
    DataType *GetNamedObject(const char *name) const
    {
      if (name == nullptr)
        return nullptr;
      DataNode *n = this->GetNamedNode(name);
      if (n == nullptr)
        return nullptr;
      else
        return dynamic_cast<DataType *>(n->GetData());
    }

    /**
     * \brief Convenience method to get the first data object of a given type with a given name (std::string overload).
     *
     * \tparam DataType  The expected data type (must inherit from BaseData).
     * \param[in] name  The node name to search for.
     * \return Pointer to the data object cast to DataType, or nullptr if not found.
     * \sa GetNamedObject(const char*)
     */
    template <class DataType>
    DataType *GetNamedObject(const std::string& name) const
    {
      return this->GetNamedObject<DataType>(name.c_str());
    }

    /**
     * \brief Convenience method to get the first data object of a given type with a given name, derived from a source node.
     *
     * \tparam DataType  The expected data type (must inherit from BaseData).
     * \param[in] name                  The node name to search for.
     * \param[in] sourceNode            The parent node to search derivations of.
     * \param[in] onlyDirectDerivations If true, only direct derivations are searched.
     * \return Pointer to the data object cast to DataType, or nullptr if not found.
     * \sa GetNamedDerivedNode
     */
    template <class DataType>
    DataType *GetNamedDerivedObject(const char *name,
                                    const DataNode *sourceNode,
                                    bool onlyDirectDerivations = true) const
    {
      if (name == nullptr)
        return nullptr;
      DataNode *n = this->GetNamedDerivedNode(name, sourceNode, onlyDirectDerivations);
      if (n == nullptr)
        return nullptr;
      else
        return dynamic_cast<DataType *>(n->GetData());
    }

    /**
     * \brief Generate a unique node name by appending an incrementing number if necessary.
     *
     * If the base \a name is already unique, it is returned as-is. Otherwise, the method
     * appends " 2", " 3", etc., filling gaps in existing numbering to maintain a compact
     * sequence. If a \a sourceNode is provided, uniqueness is checked only among its
     * derived nodes; otherwise, all nodes in the DataStorage are considered.
     *
     * \param[in] name                  The base name to make unique.
     * \param[in] sourceNode            Optional parent node restricting the search space.
     * \param[in] onlyDirectDerivations If true, only direct derivations of \a sourceNode are considered.
     * \return A unique name derived from the given base name.
     */
    std::string GetUniqueName(const std::string& name, const DataNode* sourceNode = nullptr, bool onlyDirectDerivations = true) const;

    /**
     * \brief Return all group tags used across all nodes in the DataStorage.
     *
     * Group tags are identified by the presence of GroupTagProperty entries
     * in the nodes' property lists.
     *
     * \return A set of group tag name strings.
     */
    const DataNode::GroupTagList GetGroupTags() const;

    /** \brief Mutex for thread-safe operations on the DataStorage. */
    mutable std::mutex m_MutexOne;

    /** \brief Event type used for all DataStorage notifications, carrying a const DataNode pointer. */
    typedef Message1<const DataNode*> DataStorageEvent;

    /**
     * \brief Event emitted after a new node has been added to the DataStorage.
     *
     * Register observers via AddNodeEvent.AddListener(myObject, MyObject::MyMethod).
     * Unregister via AddNodeEvent.RemoveListener(myObject, MyObject::MyMethod).
     * This event is thread-safe.
     */
    DataStorageEvent AddNodeEvent;

    /**
     * \brief Event emitted directly before a node is removed from the DataStorage.
     *
     * Register observers via RemoveNodeEvent.AddListener(myObject, MyObject::MyMethod).
     * Unregister via RemoveNodeEvent.RemoveListener(myObject, MyObject::MyMethod).
     * This event is thread-safe.
     */
    DataStorageEvent RemoveNodeEvent;

    /**
     * \brief Event emitted after a managed node has been modified.
     *
     * The DataStorage internally listens to itk::ModifiedEvent on each node and
     * forwards it through this event. Can be suppressed via BlockNodeModifiedEvents().
     *
     * Register observers via ChangedNodeEvent.AddListener(myObject, MyObject::MyMethod).
     * Unregister via ChangedNodeEvent.RemoveListener(myObject, MyObject::MyMethod).
     * This event is thread-safe.
     */
    DataStorageEvent ChangedNodeEvent;

    /**
     * \brief Event emitted directly before a managed node is deleted.
     *
     * The DataStorage internally listens to itk::DeleteEvent on each node and
     * forwards it through this event.
     *
     * Register observers via DeleteNodeEvent.AddListener(myObject, MyObject::MyMethod).
     * Unregister via DeleteNodeEvent.RemoveListener(myObject, MyObject::MyMethod).
     * This event is thread-safe.
     */
    DataStorageEvent DeleteNodeEvent;

    /**
     * \brief Event emitted when the DataInteractor of a managed node changes.
     *
     * Forwards InteractorChangedEvent from individual nodes.
     * This event is thread-safe.
     */
    DataStorageEvent InteractorChangedNodeEvent;

    /**
     * \brief Compute the axis-parallel bounding geometry encompassing the given set of nodes.
     *
     * Nodes whose specified BoolProperty is false are excluded from the computation.
     *
     * \param[in] input             Set of nodes to include. Must not be nullptr.
     * \param[in] boolPropertyKey   Optional BoolProperty key; nodes where this is false are excluded.
     * \param[in] renderer          Renderer context for property lookup.
     * \param[in] boolPropertyKey2  A second BoolProperty condition applied in addition to the first.
     * \return The computed bounding TimeGeometry, or nullptr if no valid geometries are found.
     * \throw std::invalid_argument if \a input is nullptr.
     */
    TimeGeometry::ConstPointer ComputeBoundingGeometry3D(const SetOfObjects *input,
                                                         const char *boolPropertyKey = nullptr,
                                                         const BaseRenderer *renderer = nullptr,
                                                         const char *boolPropertyKey2 = nullptr) const;

    /**
     * \brief Compute the axis-parallel bounding geometry for a set of const node pointers.
     *
     * Overload for callers that hold only DataNode::ConstPointer. Semantically identical to
     * the SetOfObjects overload.
     *
     * \param[in] input             Set of const node pointers. Must not be nullptr.
     * \param[in] boolPropertyKey   Optional BoolProperty key; nodes where this is false are excluded.
     * \param[in] renderer          Renderer context for property lookup.
     * \param[in] boolPropertyKey2  A second BoolProperty condition applied in addition to the first.
     * \return The computed bounding TimeGeometry, or nullptr if no valid geometries are found.
     * \throw std::invalid_argument if \a input is nullptr.
     */
    TimeGeometry::ConstPointer ComputeBoundingGeometry3D(const ConstSetOfObjects *input,
                                                         const char *boolPropertyKey = nullptr,
                                                         const BaseRenderer *renderer = nullptr,
                                                         const char *boolPropertyKey2 = nullptr) const;

    /**
     * \brief Compute the axis-parallel bounding geometry of all nodes in the DataStorage.
     *
     * Convenience overload that calls ComputeBoundingGeometry3D(GetAll(), ...).
     *
     * \param[in] boolPropertyKey   Optional BoolProperty key; nodes where this is false are excluded.
     * \param[in] renderer          Renderer context for property lookup.
     * \param[in] boolPropertyKey2  A second BoolProperty condition applied in addition to the first.
     * \return The computed bounding TimeGeometry.
     */
    TimeGeometry::ConstPointer ComputeBoundingGeometry3D(const char *boolPropertyKey = nullptr,
                                                         const BaseRenderer *renderer = nullptr,
                                                         const char *boolPropertyKey2 = nullptr) const;

    /**
     * \brief Compute the axis-parallel bounding geometry of all visible nodes.
     *
     * Convenience method that calls ComputeBoundingGeometry3D("visible", renderer, boolPropertyKey).
     *
     * \param[in] renderer          The renderer whose visibility property is checked.
     * \param[in] boolPropertyKey   An additional BoolProperty condition.
     * \return The computed bounding TimeGeometry of visible nodes.
     */
    TimeGeometry::ConstPointer ComputeVisibleBoundingGeometry3D(const BaseRenderer *renderer = nullptr,
                                                                const char *boolPropertyKey = nullptr);

    /**
     * \brief Compute the axis-parallel bounding box of all nodes in the DataStorage.
     *
     * \param[in] boolPropertyKey   Optional BoolProperty key; nodes where this is false are excluded.
     * \param[in] renderer          Renderer context for property lookup.
     * \param[in] boolPropertyKey2  A second BoolProperty condition applied in addition to the first.
     * \return The computed BoundingBox.
     */
    BoundingBox::Pointer ComputeBoundingBox(const char *boolPropertyKey = nullptr,
                                            const BaseRenderer *renderer = nullptr,
                                            const char *boolPropertyKey2 = nullptr);

    /**
     * \brief Compute the bounding box of all visible nodes.
     *
     * Convenience method that calls ComputeBoundingBox("visible", renderer, boolPropertyKey).
     *
     * \param[in] renderer          The renderer whose visibility property is checked.
     * \param[in] boolPropertyKey   An additional BoolProperty condition.
     * \return The computed BoundingBox of visible nodes.
     */
    BoundingBox::Pointer ComputeVisibleBoundingBox(const BaseRenderer *renderer = nullptr,
                                                   const char *boolPropertyKey = nullptr)
    {
      return ComputeBoundingBox("visible", renderer, boolPropertyKey);
    }

    /**
     * \brief Compute the time bounds of all nodes in the DataStorage.
     *
     * Returns [-infinity, +infinity] only if all data objects have infinite lifespan.
     * Otherwise, data objects with infinite lifespan are ignored, and the union of
     * all finite time bounds is returned.
     *
     * \param[in] boolPropertyKey   Optional BoolProperty key; nodes where this is false are excluded.
     * \param[in] renderer          Renderer context for property lookup.
     * \param[in] boolPropertyKey2  A second BoolProperty condition.
     * \return The computed time bounds as a TimeBounds (array of two ScalarType values).
     */
    TimeBounds ComputeTimeBounds(const char *boolPropertyKey,
                                 const BaseRenderer *renderer,
                                 const char *boolPropertyKey2);

    /**
     * \brief Compute the time bounds of all visible nodes.
     *
     * Convenience method that calls ComputeTimeBounds("visible", renderer, boolPropertyKey).
     *
     * \param[in] renderer          The renderer whose visibility property is checked.
     * \param[in] boolPropertyKey   An additional BoolProperty condition.
     * \return The computed time bounds of visible nodes.
     */
    TimeBounds ComputeTimeBounds(const BaseRenderer *renderer, const char *boolPropertyKey)
    {
      return ComputeTimeBounds("visible", renderer, boolPropertyKey);
    }

    /**
     * \brief Enable or disable suppression of ChangedNodeEvent emission.
     *
     * When \a block is true, the DataStorage will not emit ChangedNodeEvent when
     * a managed DataNode is modified. This is useful when performing batch property
     * updates and you do not want observers to react to each individual change.
     *
     * \param[in] block  If true, suppress ChangedNodeEvent; if false, re-enable it.
     * \sa ChangedNodeEvent
     */
    void BlockNodeModifiedEvents(bool block);

  protected:
    //##Documentation
    //## @brief  EmitAddNodeEvent emits the AddNodeEvent
    //##
    //## This method should be called by subclasses to emit the AddNodeEvent
    void EmitAddNodeEvent(const DataNode *node);

    //##Documentation
    //## @brief  EmitRemoveNodeEvent emits the RemoveNodeEvent
    //##
    //## This method should be called by subclasses to emit the RemoveNodeEvent
    void EmitRemoveNodeEvent(const DataNode *node);

    void OnNodeInteractorChanged(itk::Object *caller, const itk::EventObject &event);

    //##Documentation
    //## @brief  OnNodeModified listens to modified events of DataNodes.
    //##
    //## The node is hidden behind the caller parameter, which has to be casted first.
    //## If the cast succeeds the ChangedNodeEvent is emitted with this node.
    void OnNodeModifiedOrDeleted(const itk::Object *caller, const itk::EventObject &event);

    //##Documentation
    //## @brief  Adds a Modified-Listener to the given Node.
    void AddListeners(const DataNode *_Node);

    //##Documentation
    //## @brief  Removes a Modified-Listener from the given Node.
    void RemoveListeners(const DataNode *_Node);

    //##Documentation
    //## @brief  Saves Modified-Observer Tags for each node in order to remove the event listeners again.
    std::map<const DataNode *, unsigned long> m_NodeModifiedObserverTags;

    std::map<const DataNode *, unsigned long> m_NodeInteractorChangedObserverTags;

    //##Documentation
    //## @brief  Saves Delete-Observer Tags for each node in order to remove the event listeners again.
    std::map<const DataNode *, unsigned long> m_NodeDeleteObserverTags;

    //##Documentation
    //## @brief If this class changes nodes itself, set this to TRUE in order
    //## to suppress NodeChangedEvent to be emitted.
    bool m_BlockNodeModifiedEvents;

    DataStorage();
    ~DataStorage() override;

    //##Documentation
    //## @brief Filters a SetOfObjects by the condition. If no condition is provided, the original set is returned
    SetOfObjects::ConstPointer FilterSetOfObjects(const SetOfObjects *set, const NodePredicateBase *condition) const;

    //##Documentation
    //## @brief Prints the contents of the DataStorage to os. Do not call directly, call ->Print() instead
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;
  };

  /**
   * \brief Find the topmost visible node at a given world position and time point.
   *
   * Among the provided nodes, this function returns the one that is visible, not a helper
   * object, contains the given \a worldPosition within its geometry, has a valid time point,
   * and has the highest "layer" IntProperty value.
   *
   * \param[in] nodes          The candidate set of nodes. If null, returns nullptr.
   * \param[in] worldPosition  The 3D position to test for containment.
   * \param[in] timePoint      The time point that must be valid in the node's TimeGeometry.
   * \param[in] baseRender     The renderer used for visibility and layer property lookup.
   * \return The topmost visible DataNode, or nullptr if no suitable node is found.
   */
  MITKCORE_EXPORT DataNode::Pointer FindTopmostVisibleNode(const DataStorage::SetOfObjects::ConstPointer nodes,
                                                           const Point3D worldPosition,
                                                           const TimePointType timePoint,
                                                           const BaseRenderer* baseRender);
} // namespace mitk

#endif
