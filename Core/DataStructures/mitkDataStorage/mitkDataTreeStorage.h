/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKDATATREESTORAGE_H_HEADER_INCLUDED_
#define MITKDATATREESTORAGE_H_HEADER_INCLUDED_

#include "mitkDataStorage.h"
//#include "mitkCommon.h"
#include "mitkDataTree.h"

namespace mitk {

  class NodePredicateBase;
  class DataTreeNode;
  class BaseRenderer;
  class Geometry2DDataVtkMapper3D;

  //##Documentation
  //## @brief Data management class that handles 'was created by' relations
  //##
  //## The DataTreeStorage provides data storage and management functionality.
  //## It handles a 'was created by' relation by associating each data object with a
  //## set of source objects, that this object was created from. 
  //## Thus, nodes are stored in a noncyclical directed graph data structure.
  //## If a new node is added to the DataTreeStorage, AddNodeEvent is emitted. 
  //## If a node is removed, RemoveNodeEvent is emitted. 
  //## Currently, DataTreeStorage uses an underlying mitk::DataTree. Therefore a DataTreeStorage object
  //## must be initialized with a DataTree before it can be used by calling ->Initialize(myTree).
  //## @warning DataTreeStorage methods will raise exceptions if called before the DataTreeStorage is initialized.
  //##
  //## All objects that are added to DataTreeStorage get automatically added to the underlying tree.
  //## The tree can be used to get notifications on New/Delete events.
  //## @warning Do not mix Adding/Removing objects with the DataTreeStorage and DataTree methods.
  //## @warning The DataTreeStorage does not automatically return all objects that are stored in the
  //##          DataTree. Use SetManageCompleteTree() to enable/disable management of objects that were
  //##          added to the tree directly.
  //##
  //## @ingroup DataTreeStorage
  class MITK_CORE_EXPORT DataTreeStorage : public DataStorage
  {
  public:
    mitkClassMacro(DataTreeStorage, DataStorage);

    //##Documentation
    //## @brief Adds a DataTreeNode containing a data object to its internal storage
    //##
    //## This Method adds a new data object to the DataTreeStorage. The new object is
    //## passed in the first parameter. The second parameter is a set
    //## of source objects, that were used to create this object. The new object will have
    //## a 'was created from' relation to its source objects.
    //## the addition of a new object will fire the notification mechanism.
    //## If the node parameter is NULL or if the DataTreeNode has already been added,
    //## an exception will be thrown.
    void Add(mitk::DataTreeNode* node, const mitk::DataTreeStorage::SetOfObjects* parents = NULL);

    //##Documentation
    //## @brief Removes node from the DataTreeStorage
    //##
    void Remove(const mitk::DataTreeNode* node);

    //##Documentation
    //## @brief Checks if a node exists in the DataTreeStorage
    //##
    virtual bool Exists(const mitk::DataTreeNode* node) const;

    //##Documentation
    //## @brief returns a set of source objects for a given node that meet the given condition(s).
    //##
    SetOfObjects::ConstPointer GetSources(const mitk::DataTreeNode* node, const NodePredicateBase* condition = NULL, bool onlyDirectSources = true) const;

    //##Documentation
    //## @brief returns a set of derived objects for a given node.
    //##
    //## GetDerivations() returns a set of objects that are derived from the DataTreeNode node.
    //## This means, that node was used to create the returned objects. If the parameter
    //## onlyDirectDerivations is set to true (default value), only objects that directly have
    //## node as one of their source objects will be returned. Otherwise, objects that are
    //## derived from derivations of node are returned too.
    //## The derived objects can be filtered with a predicate object as described in the GetSubset()
    //## method by providing a predicate as the condition parameter.
    SetOfObjects::ConstPointer GetDerivations(const mitk::DataTreeNode* node, const NodePredicateBase* condition = NULL, bool onlyDirectDerivations = true) const;

    //##Documentation
    //## @brief returns a set of all data objects that are stored in the data storage
    //##
    SetOfObjects::ConstPointer GetAll() const;

    //##Documentation
    //## @brief Initializes the class by providing the data tree that should be used for data storage
    //##
    void Initialize(mitk::DataTree* tree);

    //##Documentation
    //## @brief Callback method to get notified, if a node in the underlying DataTree gets removed
    //##
    void NodeDeletedInTree(const itk::EventObject & treeChangedEvent);

    //##Documentation
    //## @brief If true, the DataTreeStorage object manages all objects in the dataTree, not only the ones added by it
    itkSetMacro(ManageCompleteTree, bool);
    itkGetMacro(ManageCompleteTree, bool);
    itkBooleanMacro(ManageCompleteTree);

  protected:

    //TODO investigate removing friend declarations when DataTreeStorage is
    //     independent of the DataTree
    friend class BaseRenderer;
    friend class Geometry2DDataVtkMapper3D;
    friend class RenderingManager;
    friend class DataStorage;

    itkNewMacro(Self);    // New Macro is protected, because we use Singleton pattern for DataTreeStorage

    typedef std::map<mitk::DataTreeNode::ConstPointer, SetOfObjects::ConstPointer> AdjacencyList;


    //##Documentation
    //## @brief Standard Constructor for ::New() instantiation
    DataTreeStorage();
    //##Documentation
    //## @brief Standard Destructor
    virtual ~DataTreeStorage();

    //##Documentation
    //## @brief convenience method to check if the object has been initialized (i.e. a data tree has been set)
    bool IsInitialized() const;

    //##Documentation
    //## @brief Traverses the Relation graph and extracts a list of related elements (e.g. Sources or Derivations)
    SetOfObjects::ConstPointer GetRelations(const mitk::DataTreeNode* node, const AdjacencyList& relation, const NodePredicateBase* condition = NULL, bool onlyDirectlyRelated = true) const;

    //##Documentation
    //## @brief deletes all references to a node in a given relation (used in Remove() and TreeListener)
    void RemoveFromRelation(const mitk::DataTreeNode* node, AdjacencyList& relation);

    //##Documentation
    //## @brief Prints the contents of the DataTreeStorage to os. Do not call directly, call ->Print() instead
    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;


    //##Documentation
    //## @brief holds the data tree that is encapsulated by this class
    mitk::DataTree::Pointer m_DataTree;

    //##Documentation
    //## @brief If true, the DataTreeStorage object manages all objects in the dataTree, not only the ones added by it
    bool m_ManageCompleteTree;

    //##Documentation
    //## @brief Flag that is set in Remove() method to prevent tree notification mechanism from deleting the same object we are just about to delete
    bool m_DuringRemove;

    //##Documentation
    //## @brief Nodes and their relation are stored in m_SourceNodes
    AdjacencyList m_SourceNodes;
    
    //##Documentation
    //## @brief Nodes are stored in reverse relation for easier traversal in the opposite direction of the relation
    AdjacencyList m_DerivedNodes;

    unsigned long m_DeleteInTreeObserverTag;

  };
} // namespace mitk

#endif /* MITKDATATREESTORAGE_H_HEADER_INCLUDED_ */
