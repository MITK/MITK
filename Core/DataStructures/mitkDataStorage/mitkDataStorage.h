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


#ifndef MITKDATASTORAGE_H_HEADER_INCLUDED_
#define MITKDATASTORAGE_H_HEADER_INCLUDED_

#include "itkObject.h"
#include "mitkCommon.h"
#include "mitkDataTree.h"

#include "itkVectorContainer.h"
#include <map>

namespace mitk {

  class NodePredicateBase;
  class DataTreeNode;
  class BaseRenderer;
  

  //##Documentation
  //## @brief Data management class that handles 'was created by' relations
  //##
  //## The DataStorage provides data storage and management functionality.
  //## It handles a 'was created by' relation by associating each data object with a 
  //## set of source objects, that this object was created from. The DataStorage
  //## class does not yet provide an observer mechanism to notify on changes in the data
  //## objects that it manages. This is planned for a future version.
  //## Currently, DataStorage uses an underlying mitk::DataTree. Therefore a DataStorage object
  //## must be initialized with a DataTree before it can be used by calling ->Initialize(myTree).
  //## @warning DataStorage methods will raise exceptions if called before the DataStorage is initialized.
  //##
  //## All objects that are added to DataStorage get automatically added to the underlying tree. 
  //## The tree can be used to get notifications on New/Delete events.
  //## @warning Do not mix Adding/Removing objects with the DataStorage and DataTree methods.  
  //## @warning The DataStorage does not automatically return all objects that are stored in the 
  //##          DataTree. Use SetManageCompleteTree() to enable/disable management of objects that were
  //##          added to the tree directly.  
  //## 
  //## @ingroup DataStorage
  class MITK_CORE_EXPORT DataStorage : public itk::Object
  {
  public:
    mitkClassMacro(DataStorage, itk::Object);
    //itkNewMacro(Self);

    static DataStorage* CreateInstance(mitk::DataTree* tree);  // create method that initializes singleton object
    static DataStorage* GetInstance();    // Singleton pattern like access method
    
    //##Documentation
    //## @brief A Container of objects that is used as a resultset of GetSubset() query operations (Set of SmartPointers to DataTreeNodes).
    typedef itk::VectorContainer<unsigned int, mitk::DataTreeNode::Pointer> SetOfObjects;

    //##Documentation
    //## @brief Adds a DataTreeNode containing a data object to its internal storage
    //##
    //## This Method adds a new data object to the DataStorage. The new object is 
    //## passed in the first parameter. The second parameter is a set 
    //## of source objects, that were used to create this object. The new object will have
    //## a 'was created from' relation to its source objects.
    //## the addition of a new object will fire the notification mechanism.
    //## If the node parameter is NULL or if the DataTreeNode has already been added, 
    //## an exception will be thrown.
    void Add(mitk::DataTreeNode* node, const mitk::DataStorage::SetOfObjects* parents = NULL);
    
    //##Documentation
    //## @brief Convenience method to add a node that has one parent
    //##
    void Add(mitk::DataTreeNode* node, mitk::DataTreeNode* parent);

    //##Documentation
    //## @brief Removes node from the DataStorage
    //##
    void Remove(const mitk::DataTreeNode* node);

    //##Documentation
    //## @brief Removes a set of nodes from the DataStorage
    //##
    void Remove(const mitk::DataStorage::SetOfObjects* nodes);

    //##Documentation
    //## @brief Notifies the DataStorage that the object in node has been modified
    //##
    //## Calling the Update() method will start the nofitication mechanism that will notifiy 
    //## all registered agents that the object has been updated. Depending on the registered
    //## agents, this could lead to an update of other objects, like surfaces that were created
    //## from segmentations.
    //## If the node parameter is NULL or if the DataTreeNode is not contained in the 
    //## DataStorage, an exception will be thrown.
    void Update(mitk::DataTreeNode* node);

    //##Documentation
    //## @brief returns a set of data objects that meet the given condition(s)
    //##
    //## GetSubset returns a set of objects  with a specific datatype that meet the condition(s) 
    //## specified in the condition parameter. Conditions can be 
    //##  - data type of the data object
    //##  - is source object of specific object (e.g. all source objects of node x)
    //##  - has property with specific value (e.g. OrganType is Liver)
    //##  - negation of any condition
    //##  - conjunction of a set of conditions
    //##  - disjunction of a set of conditions
    //## Conditions are implemented as predicates using the Composite Design Pattern
    //## (see definition of NodePredicateBase for details).
    //## The method returns a set of SmartPointers to the DataTreeNodes that fulfill the 
    //## conditions. A set of all objects can be retrieved with the GetAll() method;
    SetOfObjects::ConstPointer GetSubset(const NodePredicateBase& condition) const;

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
    //## @brief returns a set of all dataobjects that are stored in the data storage
    //##
    SetOfObjects::ConstPointer GetAll() const;

    //##Documentation
    //## @brief Convenience method to get the first node with a given name
    //##
    mitk::DataTreeNode* GetNamedNode(const char* name) const;
    
    //##Documentation
    //## @brief Convenience method to get the first node with a given name that is derived from sourceNode
    //##
    mitk::DataTreeNode* GetNamedDerivedNode(const char* name, const mitk::DataTreeNode* sourceNode, bool onlyDirectDerivations = true) const;

    //##Documentation
    //## @brief Convenience method to get the first data object of a given datatype with a given name
    //##
    template <class DataType>
    DataType* GetNamedObject(const char* name) const
    {
      if (name == NULL)
        return NULL;
      mitk::DataTreeNode* n = this->GetNamedNode(name);
      if (n == NULL)
        return NULL;
      else
        return dynamic_cast<DataType*>(n->GetData());
    }

    //##Documentation
    //## @brief Convenience method to get the first data object of a given datatype with a given name that is derived from a specific node
    //##
    template <class DataType>
    DataType* GetNamedDerivedObject(const char* name, const mitk::DataTreeNode* sourceNode, bool onlyDirectDerivations = true) const
    {
      if (name == NULL)
        return NULL;
      mitk::DataTreeNode* n = this->GetNamedDerivedNode(name, sourceNode, onlyDirectDerivations);
      if (n == NULL)
        return NULL;
      else
        return dynamic_cast<DataType*>(n->GetData());
    }

    //##Documentation
    //## @brief Returns a list of used grouptags
    //##
    const DataTreeNode::GroupTagList GetGroupTags() const;


    //##Documentation
    //## @brief Initializes the class by providing the data tree that should be used for data storage
    //##
    void Initialize(mitk::DataTree* tree);
    
    //##Documentation
    //## @brief Callback method to get notified, if a node in the underlying DataTree gets removed
    //##
    void NodeDeletedInTree(const itk::EventObject & treeChangedEvent);

    //##Documentation
    //## @brief If true, the DataStorage object manages all objects in the dataTree, not only the ones added by it
    itkSetMacro(ManageCompleteTree, bool);
    itkGetMacro(ManageCompleteTree, bool);
    itkBooleanMacro(ManageCompleteTree);

  protected:

    friend class BaseRenderer;

    itkNewMacro(Self);    // New Macro is protected, because we use Singleton pattern for DataStorage

    typedef std::map<mitk::DataTreeNode::ConstPointer, SetOfObjects::ConstPointer> AdjacencyList;

    //##Documentation
    //## @brief Standard Constructor for ::New() instantiation     
    DataStorage();
    //##Documentation
    //## @brief Standard Destructor
    virtual ~DataStorage();

    //##Documentation
    //## @brief convenience method to check if the object has been initialized (i.e. a data tree has been set)
    bool IsInitialized() const;

    //##Documentation
    //## @brief Filters a SetOfObjects by the condition. If no condition is provided, the original set is returned
    SetOfObjects::ConstPointer FilterSetOfObjects(const SetOfObjects* set, const NodePredicateBase* condition) const;

    //##Documentation
    //## @brief Traverses the Relation graph and extracts a list of related elements (e.g. Sources or Derivations)
    SetOfObjects::ConstPointer GetRelations(const mitk::DataTreeNode* node, const AdjacencyList& relation, const NodePredicateBase* condition = NULL, bool onlyDirectlyRelated = true) const;

    //##Documentation
    //## @brief deletes all references to a node in a given relation (used in Remove() and TreeListener)
    void RemoveFromRelation(const mitk::DataTreeNode* node, AdjacencyList& relation);

    //##Documentation
    //## @brief Prints the contents of the DataStorage to os. Do not call directly, call ->Print() instead
    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;


    //##Documentation
    //## @brief holds the data tree that is encapsulated by this class
    mitk::DataTree::Pointer m_DataTree;
    
    //##Documentation
    //## @brief If true, the DataStorage object manages all objects in the dataTree, not only the ones added by it
    bool m_ManageCompleteTree;

    //##Documentation
    //## @brief Flag that is set in Remove() method to prevent tree notification mechanism from deleting the same object we are just about to delete
    bool m_DuringRemove;

    AdjacencyList m_SourceNodes;
    AdjacencyList m_DerivedNodes;
    
    unsigned long m_DeleteInTreeObserverTag;

    static mitk::DataStorage::Pointer s_Instance;
  };
} // namespace mitk

#endif /* MITKDATASTORAGE_H_HEADER_INCLUDED_ */
