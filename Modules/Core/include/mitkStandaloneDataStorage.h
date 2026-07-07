/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStandaloneDataStorage_h
#define mitkStandaloneDataStorage_h

#include <itkVectorContainer.h>
#include <mitkDataStorage.h>
#include <mitkMessage.h>
#include <map>
#include <mutex>

namespace mitk
{
  class NodePredicateBase;
  class DataNode;

  /**
   * \brief Concrete DataStorage implementation that stores nodes in an in-memory adjacency list graph.
   *
   * StandaloneDataStorage is the standard implementation of mitk::DataStorage. It
   * manages 'was created by' relationships by maintaining two adjacency lists:
   * one for source (parent) nodes and one for derived (child) nodes. The graph
   * structure is non-cyclical and directed.
   *
   * This class is fully thread-safe; all public methods that access the internal
   * data structures are protected by a mutex.
   *
   * \ingroup DataStorage
   * \sa DataStorage, DataNode
   */
  class MITKCORE_EXPORT StandaloneDataStorage : public mitk::DataStorage
  {
  public:
    mitkClassMacro(StandaloneDataStorage, mitk::DataStorage);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Add a DataNode to the storage, optionally with parent (source) nodes.
     *
     * Inserts the node into the internal adjacency lists and establishes 'was created from'
     * relationships. Emits AddNodeEvent upon successful addition.
     *
     * \param[in] node     The DataNode to add. Must not be nullptr.
     * \param[in] parents  Optional set of parent nodes. May be nullptr.
     * \throw std::logic_error if the DataStorage is not initialized.
     * \throw std::invalid_argument if \a node is its own parent or already exists.
     * \sa Remove, DataStorage::Add
     */
    void Add(mitk::DataNode *node, const mitk::DataStorage::SetOfObjects *parents = nullptr) override;
    using DataStorage::Add; //ensure that also "void Add(DataNode *node, DataNode *parent);" is visible

    /**
     * \brief Remove a node from the StandaloneDataStorage.
     *
     * Removes the node from both adjacency lists and emits RemoveNodeEvent before removal.
     * If \a node is nullptr, the call is ignored.
     *
     * \param[in] node  The node to remove.
     * \throw std::logic_error if the DataStorage is not initialized.
     * \sa Add
     */
    void Remove(const mitk::DataNode *node) override;

    /**
     * \brief Check whether a node exists in this StandaloneDataStorage.
     *
     * \param[in] node  The node to check.
     * \return \a true if the node is present.
     */
    bool Exists(const mitk::DataNode *node) const override;

    /**
     * \brief Return the source (parent) nodes of a given node, optionally filtered.
     *
     * \param[in] node              The node whose sources are queried.
     * \param[in] condition         Optional predicate to filter results.
     * \param[in] onlyDirectSources If true (default), only direct parents are returned.
     *                              If false, all transitive sources are traversed.
     * \return A set of source DataNodes.
     * \throw std::invalid_argument if \a node is nullptr.
     * \sa GetDerivations
     */
    SetOfObjects::ConstPointer GetSources(const mitk::DataNode *node,
                                          const NodePredicateBase *condition = nullptr,
                                          bool onlyDirectSources = true) const override;

    /**
     * \brief Return the derived (child) nodes of a given node, optionally filtered.
     *
     * Returns nodes that were created from (derived from) the given \a node.
     *
     * \param[in] node                  The node whose derivations are queried.
     * \param[in] condition             Optional predicate to filter results.
     * \param[in] onlyDirectDerivations If true (default), only direct children are returned.
     *                                  If false, all transitive derivations are traversed.
     * \return A set of derived DataNodes.
     * \throw std::invalid_argument if \a node is nullptr.
     * \sa GetSources
     */
    SetOfObjects::ConstPointer GetDerivations(const mitk::DataNode *node,
                                              const NodePredicateBase *condition = nullptr,
                                              bool onlyDirectDerivations = true) const override;

    /**
     * \brief Return all nodes managed by this StandaloneDataStorage.
     *
     * \return A set containing all DataNodes.
     * \throw std::logic_error if the DataStorage is not initialized.
     */
    SetOfObjects::ConstPointer GetAll() const override;

    /** \brief Mutex protecting the internal adjacency lists for thread-safe access. */
    mutable std::mutex m_Mutex;

  protected:
    //##Documentation
    //## @brief noncyclical directed graph data structure to store the nodes with their relation
    typedef std::map<mitk::DataNode::ConstPointer, SetOfObjects::ConstPointer> AdjacencyList;

    StandaloneDataStorage();
    ~StandaloneDataStorage() override;

    //##Documentation
    //## @brief convenience method to check if the object has been initialized (i.e. a data tree has been set)
    bool IsInitialized() const;

    //##Documentation
    //## @brief Traverses the Relation graph and extracts a list of related elements (e.g. Sources or Derivations)
    SetOfObjects::ConstPointer GetRelations(const mitk::DataNode *node,
                                            const AdjacencyList &relation,
                                            const NodePredicateBase *condition = nullptr,
                                            bool onlyDirectlyRelated = true) const;

    //##Documentation
    //## @brief deletes all references to a node in a given relation (used in Remove() and TreeListener)
    void RemoveFromRelation(const mitk::DataNode *node, AdjacencyList &relation);

    //##Documentation
    //## @brief Prints the contents of the StandaloneDataStorage to os. Do not call directly, call ->Print() instead
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    //##Documentation
    //## @brief Nodes and their relation are stored in m_SourceNodes
    AdjacencyList m_SourceNodes;
    //##Documentation
    //## @brief Nodes are stored in reverse relation for easier traversal in the opposite direction of the relation
    AdjacencyList m_DerivedNodes;
  };
} // namespace mitk
#endif
