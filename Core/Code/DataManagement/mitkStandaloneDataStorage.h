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


#ifndef MITKSTANDALONEDATASTORAGE_H_HEADER_INCLUDED_
#define MITKSTANDALONEDATASTORAGE_H_HEADER_INCLUDED_

#include "mitkDataStorage.h"
#include "mitkMessage.h"
#include "itkVectorContainer.h"
#include <map>

namespace mitk {

  class NodePredicateBase;
  class DataTreeNode;

  //##Documentation
  //## @brief Data management class that handles 'was created by' relations
  //##
  //## The StandaloneDataStorage provides data storage and management functionality.
  //## It handles a 'was created by' relation by associating each data object with a
  //## set of source objects that were used to create the new object was created from. 
  //## Thus, nodes are stored in a noncyclical directed graph data structure.
  //## It is derived from mitk::DataStorage and implements its interface, 
  //## including AddNodeEvent and RemoveNodeEvent.
  //## @ingroup StandaloneDataStorage
  class MITK_CORE_EXPORT StandaloneDataStorage : public mitk::DataStorage
  {
  public:
    mitkClassMacro(StandaloneDataStorage, mitk::DataStorage);
    itkNewMacro(Self);

    //##Documentation
    //## @brief Adds a DataTreeNode containing a data object to its internal storage
    //##
    //## This Method adds a new data object to the StandaloneDataStorage. The new object is
    //## passed in the first parameter. The second parameter is a set
    //## of source objects, that were used to create this object. The new object will have
    //## a 'was created from' relation to its source objects.
    //## the addition of a new object will fire the notification mechanism.
    //## If the node parameter is NULL or if the DataTreeNode has already been added,
    //## an exception will be thrown.
    void Add(mitk::DataTreeNode* node, const mitk::DataStorage::SetOfObjects* parents = NULL);

    //##Documentation
    //## @brief Removes node from the StandaloneDataStorage
    //##
    void Remove(const mitk::DataTreeNode* node);

    //##Documentation
    //## @brief Checks if a node exists in the StandaloneDataStorage
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

  protected:

    //##Documentation
    //## @brief noncyclical directed graph data structure to store the nodes with their relation
    typedef std::map<mitk::DataTreeNode::ConstPointer, SetOfObjects::ConstPointer> AdjacencyList;

    //##Documentation
    //## @brief Standard Constructor for ::New() instantiation
    StandaloneDataStorage();
    //##Documentation
    //## @brief Standard Destructor
    virtual ~StandaloneDataStorage();

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
    //## @brief Prints the contents of the StandaloneDataStorage to os. Do not call directly, call ->Print() instead
    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

    //##Documentation
    //## @brief Nodes and their relation are stored in m_SourceNodes
    AdjacencyList m_SourceNodes;
    //##Documentation
    //## @brief Nodes are stored in reverse relation for easier traversal in the opposite direction of the relation
    AdjacencyList m_DerivedNodes;
  };
} // namespace mitk
#endif /* MITKSTANDALONEDATASTORAGE_H_HEADER_INCLUDED_ */
